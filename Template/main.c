/*!
    \file    main.c
    \brief   led spark with systick, USART print and key example

    \version 2024-03-25, V2.0.2, firmware for GD32L23x, add support for GD32L235
*/

/*
    Copyright (c) 2024, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32l23x.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "gd32l23x_eval.h"
#include "i2c_led.h"
#include "core_task.h"
#include "useBaseTime.h"
#include "sht4x_usage_demo.h"
#include "gd25qxx.h"
#define ENABLE_FWDGT (1)
#define USE_SHT4_SENSOR (1)
#define USE_IIC_LED_SENSOR (1)
#define ENABLE_UART4_PRINTF (1)
#define LED_ZIGBEE_TEST_ENABLED (0)
extern int Image$$ER_IROM1$$Base;
//
/*!
    \brief      toggle the led every 500ms
    \param[in]  none
    \param[out] none
    \retval     none
*/
#if ENABLE_UART4_PRINTF
void com_usart_init(void);
#endif
/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/

#if LED_ZIGBEE_TEST_ENABLED
void led_zigbee_test(){
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_10);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
}

void led_zigbee_run(){
    gpio_bit_set(GPIOB, GPIO_PIN_10);
    delay_1ms(500);
    delay_1ms(500);

    gpio_bit_reset(GPIOB, GPIO_PIN_10);
	delay_1ms(500);
    delay_1ms(500);
}
#endif

#ifdef ENABLE_CONTROL_CRG1
void control_crg1_zs3l_power(){
    #define CONTROL_CLK  RCU_GPIOA
    #define CONTROL_PORT GPIOA
    #define CONTROL_PIN  GPIO_PIN_15
    rcu_periph_clock_enable(CONTROL_CLK);
    /* configure LED GPIO pin */
    gpio_mode_set(CONTROL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CONTROL_PIN);
    gpio_bit_set(CONTROL_PORT, GPIO_PIN_15);
}
#endif

int main(void)
{
    nvic_vector_table_set(FLASH_BASE,(uint32_t)&Image$$ER_IROM1$$Base - FLASH_BASE);
    /* configure systick */
    systick_config();
    #if ENABLE_FWDGT
        rcu_osci_on(RCU_IRC32K);
        /* wait till IRC40K is ready */
        rcu_osci_stab_wait(RCU_IRC32K);
    #endif

    #if LED_ZIGBEE_TEST_ENABLED
        led_zigbee_test();
    #endif
    delay_1ms(500);
    com_usart_init();

    /* print out the clock frequency of system, AHB, APB1 and APB2 */
    //printf("CK_SYS is %d\r\n", rcu_clock_freq_get(CK_SYS));
    IIC2_All_Init();
    useGeneralTimerProcess(11,5000);
    #if USE_SHT4_SENSOR
        delay_1ms(10);
        Init_SHT4X();
        // //初始化温湿度传感
        if(get_sht4x_status() != STATUS_OK){
            i2c_deinit(I2C0);
            IIC2_All_Init();
        }
    #endif
    #if USE_IIC_LED_SENSOR
        IS31FL3236_Init();
    #endif

    spi_flash_init();
    delay_1ms(500);
    #if ENABLE_FWDGT
        if(SUCCESS == fwdgt_config(625*3, FWDGT_PSC_DIV64)){
            fwdgt_write_disable();
            fwdgt_enable();
            printf("fwdgt_enable\r\n");
        }
    #endif
    printf("spi_flash_read_id:0x%x\r\n",spi_flash_read_id());
    while(1)
    {
        #if LED_ZIGBEE_TEST_ENABLED
        led_zigbee_run();
        #endif
        measure_task_exe();
        #if ENABLE_FWDGT
            fwdgt_write_enable();
            fwdgt_counter_reload();
        #endif
    }
}

#if ENABLE_UART4_PRINTF
void com_usart_init(void)
{
    /* enable COM GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_UART4);

    /* connect port to USART TX */
    gpio_af_set(GPIOB, GPIO_AF_8, GPIO_PIN_3);
    /* connect port to USART RX */
    gpio_af_set(GPIOB, GPIO_AF_8, GPIO_PIN_4);

    /* configure USART TX as alternate function push-pull */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_3);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_3);

    /* configure USART RX as alternate function push-pull */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_4);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_4);

    /* USART configure */
    usart_deinit(UART4);
    usart_word_length_set(UART4, USART_WL_8BIT);
    usart_stop_bit_set(UART4, USART_STB_1BIT);
    usart_parity_config(UART4, USART_PM_NONE);
    usart_baudrate_set(UART4, 115200U);
    usart_receive_config(UART4, USART_RECEIVE_ENABLE);
    usart_transmit_config(UART4, USART_TRANSMIT_ENABLE);

    usart_enable(UART4);
}
/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(UART4, (uint8_t)ch);
    while(RESET == usart_flag_get(UART4, USART_FLAG_TBE)) {
    }

    return ch;
}
#endif