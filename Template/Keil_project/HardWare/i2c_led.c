/*!
    \file    i2c.c
    \brief   slave receiver


    \version 2020-12-12, V2.1.0, firmware for GD32L235
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

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
#include "main_debug.h"

#ifdef GD32L235
#include "gd32l23x.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include "systick.h"
#include "i2c_led.h"

#if SELF_DELAY_API
#include "sys.h"
#endif

#ifdef GD32L235
#include "gd32l23x_eval.h"
#endif


#define I2C0_OWN_ADDRESS7      0x82
#define I2C1_SLAVE_ADDRESS7    0x72
#define AD_GND 0x78
#define AD_SCL 0x7a
#define AD_VCC 0x7e
#define AD_SDA 0x7c

#define I2C0_SLAVE_ADDRESS7    AD_GND
#define IS31FL3236A_SD      0x00
#define IS31FL3236A_UPDATE  0x25
#define IS31FL3236A_CTRL    0x4A
#define IS31FL3236A_FREQ    0x4B
#define IS31FL3236A_RESET   0x4F

//add 


//temperature humidity percent  0x1d
//#define WIFI_CHANLE_POS_VALUE 0x1e
//#define DEGREE_SYMBOL_CHANLE_POS_VALUE 0x1f
//degree upper part 0x20
//degree next  part 0x21
//temperature decimal point     温度 小数点 0x22
//humidity    decimal point     湿度小数点符号 0x23

//数字显示块,ID0.ID1 为温度显示，ID2.ID3为湿度显示
typedef enum {
    NUMBER_ID0=0,
    NUMBER_ID1,
    NUMBER_ID2,
    NUMBER_ID3
}NUMBER_ID_MAP_E;

//add
I2C_LED_ATTR_S      local_i2c_led_value;
IIC_BUS_STATUS      i2c_bus_status = BUS_IDLE;
static  uint8_t     i2c_led_level = 0;

//led灯亮度曲线
uint8_t  PWM_Gamma64[64]=
{
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
  0x08,0x09,0x0b,0x0d,0x0f,0x11,0x13,0x16,
  0x1a,0x1c,0x1d,0x1f,0x22,0x25,0x28,0x2e,
  0x34,0x38,0x3c,0x40,0x44,0x48,0x4b,0x4f,
  0x55,0x5a,0x5f,0x64,0x69,0x6d,0x72,0x77,
  0x7d,0x80,0x88,0x8d,0x94,0x9a,0xa0,0xa7,
  0xac,0xb0,0xb9,0xbf,0xc6,0xcb,0xcf,0xd6,
  0xe1,0xe9,0xed,0xf1,0xf6,0xfa,0xfe,0xff
};
//led显示数字码表0-9
uint8_t digital_code[10]=
{
    0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f
};

void rcu_config(void);
void gpio_config(void);
void i2c_config(void);

static void delay1s(uint8_t n){
    uint8_t i = 0;
    for(i=0;i< n;i++){
        delay_1ms(500);
        delay_1ms(500);
    }
}

/*!
    \brief      enable the peripheral clock
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    /* enable GPIOF clock */
    //rcu_periph_clock_enable(RCU_GPIOA);//RCU_GPIOF
	rcu_periph_clock_enable(RCU_GPIOB);
    /* enable I2C0 clock */
    rcu_periph_clock_enable(RCU_I2C0);
}

/*!
    \brief      configure the GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
*/
void gpio_config(void)
{
    /* connect PB8 to I2C0_SCL */
    gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_8);
    /* connect PB9 to I2C0_SDA */
    gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_9);
    /* configure GPIO pins of I2C0 */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_8);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_9);
}

/*!
    \brief      configure the I2C0 interface
    \param[in]  none
    \param[out] none
    \retval     none
*/
void i2c_config(void)
{
    #ifdef GD32E230
    /* I2C clock configure */
    i2c_clock_config(I2C1, 100000, I2C_DTCY_2);

    /* I2C address configure */
    i2c_mode_addr_config(I2C1, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS,I2C0_OWN_ADDRESS7);
    /* enable I2C1 */
    i2c_enable(I2C1);
    /* enable acknowledge */
    i2c_ack_config(I2C1, I2C_ACK_ENABLE);
    #endif

    #ifdef GD32L235
    rcu_i2c_clock_config(IDX_I2C0, RCU_I2CSRC_CKAPB1);
    /* configure I2C timing */
    i2c_timing_config(I2C0, 0x03, 0x3, 0);
    i2c_master_clock_config(I2C0, 0x13, 0x32);
    i2c_address_config(I2C0, I2C0_OWN_ADDRESS7, I2C_ADDFORMAT_7BITS);
    /* configure slave address */
    //i2c_master_addressing(I2C0, 0x82, I2C_MASTER_TRANSMIT);
    // /* configure number of bytes to be transferred */
    //i2c_transfer_byte_number_config(I2C0, 16);
    /* enable I2C0 */
    i2c_enable(I2C0);
    #endif
}

void i2c_bus_reset()
{
    #define I2C_SCL_PORT    GPIOB
    #define I2C_SDA_PORT    GPIOB
    #define I2C_SCL_PIN     GPIO_PIN_8
    #define I2C_SDA_PIN     GPIO_PIN_9
    /* configure SDA/SCL for GPIO */
    GPIO_BC(I2C_SCL_PORT) |= I2C_SCL_PIN;
    GPIO_BC(I2C_SCL_PORT) |= I2C_SDA_PIN;
    /* reset PB8 and PB9 */
    gpio_mode_set(I2C_SCL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, I2C_SCL_PIN);
    gpio_output_options_set(I2C_SCL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, I2C_SCL_PIN);
    gpio_mode_set(I2C_SDA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, I2C_SDA_PIN);
    gpio_output_options_set(I2C_SDA_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, I2C_SDA_PIN);
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    /* stop signal */
    GPIO_BOP(I2C_SCL_PORT) |= I2C_SCL_PIN;
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    GPIO_BOP(I2C_SDA_PORT) |= I2C_SDA_PIN;
    /* connect PB8 to I2C_SCL */
    /* connect PB9 to I2C_SDA */
    gpio_mode_set(I2C_SCL_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C_SCL_PIN);
    gpio_output_options_set(I2C_SCL_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, I2C_SCL_PIN);
    gpio_mode_set(I2C_SDA_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C_SDA_PIN);
    gpio_output_options_set(I2C_SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, I2C_SDA_PIN);
}

//IIC的初始化(配置时钟,模式,gpio等)
void IIC2_All_Init(){//i2c_init
    rcu_config();
    gpio_config();
    i2c_config();
}

//add 2022-03-17
//设置led灯pwm(电平值)
void set_led_level(uint8_t level){
    if(level > 0xff)return;
    i2c_led_level = level;
}
//得到当前led灯pwm(电平值)
uint8_t get_current_led_level(){
    return i2c_led_level;
}
//end

//写byte 带超时
uint8_t  IS31_WriteOneByte_timeout(uint8_t WriteAddr,uint8_t DataToWrite){
    #ifdef GD32E230
    uint8_t    state = I2C_START;
    uint16_t   timeout = 0;
    uint8_t    i2c_timeout_flag = 0;
    while(!(i2c_timeout_flag)){
        switch (state)
        {
        case I2C_START:
            while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            if(timeout < I2C_TIME_OUT){
                i2c_start_on_bus(I2C0);
                timeout = 0;
                state = I2C_SEND_ADDRESS;
            }else{
                //i2c_software_reset_config(I2C1,I2C_SRESET_SET);
                timeout = 0;
                state = I2C_START;
                i2c_timeout_flag = I2C_OK;
                #if (DEBUG_MODE)
                PR_ERR("i2c bus is busy in  WRITE!\r\n");
                #endif
            }
            break;
        case I2C_SEND_ADDRESS:
            while((!i2c_flag_get(I2C1, I2C_FLAG_SBSEND)) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            if(timeout < I2C_TIME_OUT){
                i2c_master_addressing(I2C1, I2C0_SLAVE_ADDRESS7, I2C_TRANSMITTER);
                timeout = 0;
                state = I2C_CLEAR_ADDRESS_FLAG;
            }else{
                timeout = 0;
                state = I2C_START;
                i2c_timeout_flag = I2C_OK;
                #if (DEBUG_MODE)
                PR_ERR("i2c master sends start signal timeout in  WRITE!\r\n");
                #endif
            }
            break;
        case I2C_CLEAR_ADDRESS_FLAG:
            /* address flag set means i2c slave sends ACK */
            while((!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) && (timeout < I2C_TIME_OUT)){
                timeout++; 
            }
            if(timeout < I2C_TIME_OUT){
                i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
                timeout = 0;
                state = I2C_TRANSMIT_DATA;
            }else{
                timeout = 0;
                state = I2C_START;
                i2c_timeout_flag = I2C_OK;
                #if (DEBUG_MODE)
                PR_ERR("i2c master clears address flag timeout in  WRITE!\r\n");
                #endif
            }
            break;
        case I2C_TRANSMIT_DATA:
            while((!i2c_flag_get(I2C0, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            if(timeout < I2C_TIME_OUT){
                i2c_data_transmit(I2C0, WriteAddr);
                timeout = 0;
            }else{
                timeout = 0;
                state = I2C_START;
                #if (DEBUG_MODE)
                PR_ERR("i2c master sends 's internal address timeout in  WRITE!\n");
                #endif
            }
            while((!i2c_flag_get(I2C0, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            if(timeout < I2C_TIME_OUT){
                i2c_data_transmit(I2C0, DataToWrite);
                timeout = 0;
                // while((!i2c_flag_get(I2C1, I2C_FLAG_BTC)) && (timeout < I2C_TIME_OUT)){
                //     timeout++;
                // }
                if(timeout < I2C_TIME_OUT){
                    timeout = 0;
                    state = I2C_STOP;
                }else{
                    timeout = 0;
                    state = I2C_START;
                    #if (DEBUG_MODE)
                    PR_ERR("i2c master sends data timeout in  WRITE! \r\n");
                    #endif
                }
            }else{
                timeout = 0;
                state = I2C_START;
                #if (DEBUG_MODE)
                PR_ERR("i2c master sends data timeout in  WRITE! \r\n");
                #endif
            }
            break;
        case I2C_STOP:
            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(I2C0);
            /* i2c master sends STOP signal successfully */
            while((I2C_CTL1(I2C0) & 0x0200) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            if(timeout < I2C_TIME_OUT){
                timeout = 0;
                state = I2C_END;
                i2c_timeout_flag = I2C_OK;
            }else{
                timeout = 0;
                state   = I2C_START;
                #if (DEBUG_MODE)
                PR_ERR("i2c master sends stop signal timeout in  WRITE!\r\n");
                #endif
            }
            break;
        default:
            state = I2C_START;
            i2c_timeout_flag = I2C_OK;
            timeout = 0;
            #if (DEBUG_MODE)
            PR_DEBUG("i2c master sends start signal in  WRITE!\r\n");
            #endif
            break;
        }
    }
    #endif
    #ifdef GD32L235
    i2c_process_enum    state = I2C_START;
    uint16_t   timeout = 0;
    uint8_t    end_flag = 0;
    i2c_transfer_byte_number_config(I2C0, 16);
    while(!(end_flag)){
        switch(state){
            case I2C_START:
                while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT)){
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT){
                    i2c_start_on_bus(I2C0);
                    timeout = 0;
                    state = I2C_SEND_ADDRESS;
                }else{
                    timeout = 0;
                    state = I2C_START;
                    end_flag = I2C_OK;
                    printf("i2c bus is busy in  WRITE! %d\r\n",__LINE__);
                }
                break;
            case I2C_SEND_ADDRESS:
                i2c_master_addressing(I2C0, I2C0_SLAVE_ADDRESS7, I2C_MASTER_TRANSMIT);
                timeout = 0;
                state = I2C_TRANSMIT_DATA;
                // while((!i2c_flag_get(I2C0, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)){
                //     timeout++; 
                // }
                // if(timeout < I2C_TIME_OUT){

                //     timeout = 0;
                //     state = I2C_TRANSMIT_DATA;
                // }else{
                //     timeout = 0;
                //     state = I2C_START;
                //     end_flag = I2C_OK;
                //     printf("i2c master clear address flag timeout in  WRITE!\r\n");
                // }
                break;
            case I2C_TRANSMIT_DATA:
                while((!i2c_flag_get(I2C0, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)){
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT){
                    i2c_data_transmit(I2C0, WriteAddr);
                    timeout = 0;
                }else{
                    timeout = 0;
                    state = I2C_START;
                    #if (DEBUG_MODE)
                    PR_ERR("i2c master sends 's internal address timeout in  WRITE!\n");
                    #endif
                }
                while((!i2c_flag_get(I2C0, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)){
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT){
                    i2c_data_transmit(I2C0, DataToWrite);
                    timeout = 0;
                    if(timeout < I2C_TIME_OUT){
                        timeout = 0;
                        state = I2C_STOP;
                    }else{
                        timeout = 0;
                        state = I2C_START;
                        #if (DEBUG_MODE)
                        PR_ERR("i2c master sends data timeout in  WRITE! \r\n");
                        #endif
                    }
                }else{
                    timeout = 0;
                    state = I2C_START;
                    #if (DEBUG_MODE)
                    PR_ERR("i2c master sends data timeout in  WRITE! \r\n");
                    #endif
                }
                break;
            case I2C_STOP:
                /* send a stop condition to I2C bus */
                i2c_stop_on_bus(I2C0);
                /* i2c master sends STOP signal successfully */
                while(!i2c_flag_get(I2C0, I2C_FLAG_STPDET) && (timeout < I2C_TIME_OUT)){
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT){
                    timeout = 0;
                    state = I2C_END;
                    end_flag = I2C_OK;
                    /* clear the STPDET bit */
                    i2c_flag_clear(I2C0, I2C_FLAG_STPDET);
                }else{
                    timeout = 0;
                    state   = I2C_START;
                    #if (DEBUG_MODE)
                    PR_ERR("i2c master sends stop signal timeout in  WRITE!\r\n");
                    #endif
                }
                break;
        default:
            state = I2C_START;
            end_flag = I2C_OK;
            timeout = 0;
            printf("i2c master sends start signal in  WRITE!\r\n");
            break;
        }
    }
    #endif
    return state;//if return I2C_END tx ok,else fail
}

void setReset(){
    uint8_t Is31_write_flag = 0xff;
    Is31_write_flag = IS31_WriteOneByte_timeout(IS31FL3236A_RESET,0x00);
}

void setPower(uint8_t flag){
    uint8_t Is31_write_flag = 0xff;
    if(flag == 0){
        Is31_write_flag = IS31_WriteOneByte_timeout(IS31FL3236A_SD,0x00);
    }else if(flag == 1){
        Is31_write_flag = IS31_WriteOneByte_timeout(IS31FL3236A_SD,0x01);
    }
}
static LED_RUN_MODE s_led_run_mode = LED_NORMAL;//add:11-18
// IS31FL3236A_CTRL;Global Control Register:Set all channels enable
void setRunMode(LED_RUN_MODE mode){
    //0: Normal operation
    //1: Shutdown all LEDs
    s_led_run_mode = mode;//add:11-18
    uint8_t Is31_write_flag = 0xff;
    Is31_write_flag =  IS31_WriteOneByte_timeout(IS31FL3236A_CTRL,mode==LED_NORMAL?0:0xff);
}

//add:11-18
LED_RUN_MODE Get_led_run_mode(){
    return s_led_run_mode;
}

//频率 IS31FL3236A_FREQ:Output frequency setting register
void setFreq(uint8_t freq){
    uint8_t Is31_write_flag = 0xff;
    if(freq == 3){
        Is31_write_flag = IS31_WriteOneByte_timeout(IS31FL3236A_FREQ,0x00);
    }else if(freq == 22){
        Is31_write_flag = IS31_WriteOneByte_timeout(IS31FL3236A_FREQ,0x01);//set pwm 
    }
}

void setPwm(uint8_t channel,uint8_t level){
    uint8_t Is31_write_flag = 0xff;
    Is31_write_flag= IS31_WriteOneByte_timeout(channel,level);//channel is (0x01 -0x24) . set pwm level (0-255)
}

static uint8_t m_IS31FL_status = IS31FL_UNKOWN;

uint8_t get_IS31FL_status(void){
    return m_IS31FL_status;
}

void  IS31FL3236_Init(){
    // choices are 0x00 (IMax = 23 mA), 0x01 (IMax/2), 0x02 (IMax/3), and 0x03 (IMax/4)
    uint8_t m_outCurrent = 0x02;
    uint8_t _outCurrent = m_outCurrent;
    uint8_t i = 0;
    uint8_t Is31_write_flag = 0xff;

    setRunMode(LED_NORMAL);
    //setFreq(22);
    //往26h~49h地址更新：LED Control Register;
    //0x00 Led off
    //0x01 Led on
    for(i=0x26;i<=0x49;i++){//电流 大小以及 开关设定(地址)
        if(i == 0x42){
            _outCurrent = 0x00;//只有该通道 电流最大,其余通道为最大的1/3
        }else{
            _outCurrent = m_outCurrent;
        }
        Is31_write_flag = IS31_WriteOneByte_timeout(i, (_outCurrent << 1) | 0x01); //enable all leds
        if(Is31_write_flag != I2C_END){
            m_IS31FL_status = IS31FL_FAIL;
            PR_ERR("Init_IS31FL_status fail!\r\n");
            return;
        }
    }
    //set PWM Register// 亮度调节
    for(i=0x01;i<= 0x24;i++){
        if(i == Wifi_Channle_Pos_Value){
            Is31_write_flag = IS31_WriteOneByte_timeout(i,0);//set pwm for all leds to 0
        }else if(i == Net_Status_Indicator){
            Is31_write_flag = IS31_WriteOneByte_timeout(i,0);//set pwm for all leds to 0
        }else{
            Is31_write_flag = IS31_WriteOneByte_timeout(i,0xff);//set pwm for all leds to 0
        }
    }
    
    //IS31FL3236A_UPDATE :Update Register
    Is31_write_flag = IS31_WriteOneByte_timeout(IS31FL3236A_UPDATE,0x00);//update led,pwm control reg
    //IS31FL3236A_SD:Shutdown Register-->normal mode
    Is31_write_flag = IS31_WriteOneByte_timeout(IS31FL3236A_SD,0x01);//set bit 0 to enable
    
    init_led_status();
    set_led_level(PWM_Gamma64[58]);//设置led为最大pwm曲线值
    m_IS31FL_status = IS31FL_OK;
    #if (DEBUG_MODE)
        PR_DEBUG("Init_IS31FL_status OK!\r\n");
    #endif 
}

void setPwmupdate(){
    uint8_t Is31_write_flag = 0xff;
    Is31_write_flag = IS31_WriteOneByte_timeout(IS31FL3236A_UPDATE,0x00);
}

void update_data_prepare(){
    uint8_t i =0;
    uint8_t Is31_write_flag = 0xff;
    for(i=0x01;i<= 0x24;i++){
        Is31_write_flag = IS31_WriteOneByte_timeout(i,0x00);//set pwm for all leds to 0
    }
    Is31_write_flag = IS31_WriteOneByte_timeout(IS31FL3236A_UPDATE,0x00);//update led,pwm control reg
    Is31_write_flag = IS31_WriteOneByte_timeout(IS31FL3236A_SD,0x01);//set bit 0 to enable
}

void update_data_prepare_number(){
    uint8_t i =0;
    uint8_t Is31_write_flag = 0xff;
    for(i=0x01;i<= 0x1C;i++){
        Is31_write_flag = IS31_WriteOneByte_timeout(i,0x00);//set pwm for all leds to 0
    }
    Is31_write_flag =IS31_WriteOneByte_timeout(IS31FL3236A_UPDATE,0x00);//update led,pwm control reg
}

//显示状态
//温湿度图标(百分比),% 符号,摄(华)氏度符号
void led_dispaly_state(Led_Point_Channle_Map m,LED_RUN_MODE mode){
    uint8_t level = 0;
    if(mode == LED_NORMAL){//led亮
        level = i2c_led_level;
    }
    switch(m)
    {
    case Temp_Hum_Percent:
        setPwm(Temp_Hum_Percent,level);
        break;
    case Net_Status_Indicator:
        setPwm(Net_Status_Indicator,level);
        break;
    case Degree_Symbol_Centigrade_Part:
        setPwm(Degree_Symbol_Centigrade_Part,level);
        break;
    case Degree_Symbol_Fahrenheit_Part:
        setPwm(Degree_Symbol_Fahrenheit_Part,level);
        break;
    default:
        break;
    }
    setPwmupdate();
}

//add : IR status
//增加红外(发射)状态提示
void showIR_status(uint8_t ir_status){
    setPwm(IR_Status_Pos,0);
    setPwmupdate();
    if(ir_status){
        setPwm(IR_Status_Pos,i2c_led_level);
        setPwmupdate();
    }
}

//add
void show_negative(uint8_t symbol){
    if(symbol== 0){//关闭
        setPwm(IR_Status_Pos,0);
    }else{//打开
        setPwm(IR_Status_Pos,i2c_led_level);
    }
    setPwmupdate();
}

uint8_t showNumber(const uint8_t number,NUMBER_ID_MAP_E number_id,uint8_t pwm_level){
    int i = 0;
    int j = 0;
    uint8_t channel_begin = 0;
    uint8_t channel_end   = 0;
    switch (number_id)
    {
    case NUMBER_ID0:
        channel_begin = 0x01;
        channel_end   = 0x07;
        break;
    case NUMBER_ID1:
        channel_begin = 0x08;
        channel_end   = 0x0E;
        break;
    case NUMBER_ID2:
        channel_begin = 0x0F;
        channel_end   = 0x15;
        break;
    case NUMBER_ID3:
        channel_begin = 0x16;
        channel_end   = 0x1C;
        break;
    default:
        break;
    }
    
    if(pwm_level > 255){
        return 1;
    }
    uint8_t temp = 0;
    
    for(i=channel_begin;i<= channel_end;i++){
        temp = (digital_code[number] >> j)& 1;
        if(temp){
            setPwm(i,pwm_level);//update led refresh issue;2023-01-07
        }
        j++;    
    }
    
    return 0;
}

static uint8_t I2C_CTRL_Show_half(uint8_t id,int16_t value){
    if(value < 0) value = abs(value);
    uint8_t temp = value%10;
    #if 0
    if(temp >=5){
        //show .
        setPwmupdate();
        setPwm(id,i2c_led_level);
        //PR_NOTICE("###tempMax,ok### %d\r\n",temperature);
    }else{
        setPwmupdate();
        setPwm(id,0);
        //PR_NOTICE("###tempMin,ok### %d\r\n",temperature);
    }
    #endif
    //0.3-0.7 修正:show 11-26
    setPwmupdate();//update led refresh issue
    if(temp >= 3 && (temp <= 7)){
        setPwm(id,i2c_led_level);
    }else if(temp < 3){
        setPwm(id,0);
    }else{
        setPwm(id,0);
        return 1;
    }
    return 0;
}

//更新温湿度数据(包括更新小数点)
//input params:
//@ temperature: 0-400,修正后-15-500,传入的temp参数已经扩大10倍
//@ humidity : 实际显示范围在0-99 ,传入的humi参数已经扩大10倍0-1000,例如:实际是80.5,那么该参数是805
void I2C_CTRL_Show_temp_humi(int16_t temp,uint16_t humi){
    //计算温度是否大于等于.5
    int16_t     temperature = temp;
    uint16_t    humidity = humi;
    uint8_t     update = 0;//issuse 2023-01-09
    static      uint16_t old_temper = 0;
    static      uint16_t old_humi = 0;
    #define NEGATIVE_ENABLE 1
    const uint16_t humi_max = 993;//最大湿度
    uint8_t show_half_temp = 0;
    uint8_t show_half_humi = 0;

    #if NEGATIVE_ENABLE
        show_half_temp = I2C_CTRL_Show_half(Temp_Decimal_Point,temperature);
    #else
        if(temperature < 0){
            show_half_temp = I2C_CTRL_Show_half(Temp_Decimal_Point,0);//小于0°时, .5 不显示
        }else {
            show_half_temp = I2C_CTRL_Show_half(Temp_Decimal_Point,temperature);//第5次温度判定
        }
    #endif

    if(humidity >= humi_max){
        show_half_humi = I2C_CTRL_Show_half(Humi_Decimal_Point,humi_max);//湿度超过最大显示,湿度.5显示
    }else if(humidity <= 10){
        show_half_humi = I2C_CTRL_Show_half(Humi_Decimal_Point,0);// 小于1%时, .5 不显示
    }else{
        show_half_humi = I2C_CTRL_Show_half(Humi_Decimal_Point,humidity);//正常.第二次湿度判定
    }

    temperature = temperature/10;
    humidity = humidity/10;

    //add:如果返回值等于1,(+1):11-26
    if(show_half_temp)temperature= temperature+1;
    if(show_half_humi)humidity   = humidity+1;

    //issuse 2023-01-09
    if((old_temper == temperature) && (old_humi == humidity)){
        update = 0;
    }
    if(old_temper != temperature){
        old_temper = temperature;
        update = 1;
    }
    if(old_humi != humidity){
        old_humi = humidity;
        update = 1;
    }

    //计算十位.个位.部分
    uint8_t temp_low     = temperature%10;
    uint8_t temp_hight   = temperature/10%10;
    uint8_t humi_low     = humidity%10;
    uint8_t humi_hight   = humidity/10%10;
    
    if(update){
        show_negative(0);
        update_data_prepare_number();
        if(temperature >= 99){//当本地显示华氏度数值大于99无法显示,issue:2023-01-14,对于已经超出范围的,按本地最大显示
            showNumber(9,NUMBER_ID0,i2c_led_level);
            showNumber(9,NUMBER_ID1,i2c_led_level);
        }else if(temperature < 0){
            #if NEGATIVE_ENABLE
                temp_low     = abs(temperature)%10;
                temp_hight   = abs(temperature)/10%10;
                showNumber(temp_hight,NUMBER_ID0,i2c_led_level);
                showNumber(temp_low,NUMBER_ID1,i2c_led_level);
                //if negative,show negative
                show_negative(1);
            #else
                showNumber(0,NUMBER_ID0,i2c_led_level);
                showNumber(0,NUMBER_ID1,i2c_led_level);
            #endif
        }else{//正常显示温度
            showNumber(temp_hight,NUMBER_ID0,i2c_led_level);
            showNumber(temp_low,NUMBER_ID1,i2c_led_level);
        }
        if(humi <= 10){
            showNumber(0,NUMBER_ID2,i2c_led_level);
            showNumber(1,NUMBER_ID3,i2c_led_level);
        }else {
            showNumber(humi_hight,NUMBER_ID2,i2c_led_level);
            showNumber(humi_low,NUMBER_ID3,i2c_led_level);
        }
        setPwmupdate();//update led refresh issue
    }
}
//led显示数字(number:1只显示数字,0,led全控制)
void led_show_number(uint8_t number){
    int i = 0;
    int j = 0;
    uint8_t isEnd = 0x24;
    if(number==1){
        isEnd = 0x1C;
    }
    
		
    for(j=0x01;j<= isEnd;j++)
    {
       setPwm(j,PWM_Gamma64[63]);
    }
    IS31_WriteOneByte_timeout(IS31FL3236A_UPDATE,0x00);
}
//初始化led状态
void init_led_status(){
    local_i2c_led_value.status = TurnOff;
	local_i2c_led_value.level  = PWM_Gamma64[0];//led level 为0,熄灭
}
//设置wifi_led状态，staus状态值，levels电平值
void set_wifiled_status(LedStatus status,uint8_t levels){
    local_i2c_led_value.status = status;
    local_i2c_led_value.level  =  levels;
}

static uint8_t humi;
static uint8_t temp;

#ifdef IIC_LED_TEST_DEMO
int i2cled_test_process(){
    uint8_t dht_ret = 0;
    #if SELF_DELAY_API
        delay_systick_config();
    #else
        systick_config();
    #endif

    gd_eval_com_init(EVAL_COM1);
    printf(" IIC2_All_Init begin \r\n");
    IIC2_All_Init();
    IS31FL3236_Init();
    printf(" IS31FL3236_Init end \r\n");

    led_show_number(0);

    delay_1ms(500);
    delay_1ms(500);
    delay_1ms(500);
    delay_1ms(500);

    uint16_t b = 0x1d;
    for(b = 0x1d;b< 0x25;b++){
        setPwm(b,64);
        setPwmupdate();
        delay_1ms(500);
        delay_1ms(500);
        delay_1ms(500);
        delay_1ms(500);
    }
    
    while(1)
    {
        #if DHT_API
        if(dht_ret == 0){
            dht_ret = DHT11_Init();
        }
        if(dht_ret){
            delay_1ms(500);
            delay_1ms(500);
            delay_1ms(500);
            delay_1ms(500);
            dht_ret = DHT11_get_zvalue(&humi,&temp);
            if(dht_ret){
                uint8_t temp_low = temp%10;
                uint8_t temp_hight = temp/10%10;
                uint8_t humi_low = humi%10;
                uint8_t humi_hight = humi/10%10;
                
                update_data_prepare();

                printf("\r\n temp:%d",temp);
                printf("\r\n humi:%d",humi);
            }
        }else{
            printf("\r\n detect dht,failed");
        }
        #endif

        //printf("\r\n hello");
        delay_1ms(500);
        delay_1ms(500);
    }
    return 0;
}
#endif

uint8_t led_wifi_status(I2C_LED_ATTR_S attr){
    //if i2c_led driver fail,return 
    if(attr.level == 0){
        return 1;//if level value is 0,return 
    }
    static __IO uint8_t  states = 0xff;
    static __IO uint32_t timingdelaylocal = 0U;
    static __IO uint32_t timingdelaylocal_slow = 0U;
    uint16_t base = 1;
    if( i2c_bus_status == BUS_IDLE){
        i2c_bus_status = BUS_BUSY;
        switch(attr.status)
        {
        case TurnOn:
            setPwm(Wifi_Channle_Pos_Value,attr.level);
            setPwmupdate();
            break;
        case LightingQuick://亮灭各0.5s
            base = base *1;
            if(timingdelaylocal){
                if(timingdelaylocal < base*5U){
                    setPwm(Wifi_Channle_Pos_Value,attr.level);
                    setPwmupdate();
                }else{
                    setPwm(Wifi_Channle_Pos_Value,0);
                    setPwmupdate();
                }
                timingdelaylocal--;
            }else{
                timingdelaylocal = base*10U;
            }
            break;
        case LightingSlow://亮灭各1s
            base = base *2;
            if(timingdelaylocal_slow){
                if(timingdelaylocal_slow < base*5U){
                    setPwm(Wifi_Channle_Pos_Value,attr.level);
                    setPwmupdate();
                }else{
                    setPwm(Wifi_Channle_Pos_Value,0);
                    setPwmupdate();
                }
                timingdelaylocal_slow--;
            }else{
                timingdelaylocal_slow = base*10U;
            }
            break;
        case TurnOff:
            setPwm(Wifi_Channle_Pos_Value,0);
            setPwmupdate();
            break;
        default:
            break;
        }
        i2c_bus_status = BUS_IDLE;
    }
    states = attr.status;
    return 0;
}

//add
#if 0 
uint8_t led_remote_status(I2C_LED_ATTR_S attr){
    //if i2c_led driver fail,return 
    if(attr.level == 0){
        return 1;//if level value is 0,return 
    }
    static __IO uint32_t states = 0xff;
    static __IO uint32_t timingdelaylocal = 0U;
    static __IO uint32_t timingdelaylocal_slow = 0U;
    uint16_t base = 1;
    if( i2c_bus_status == BUS_IDLE){
        i2c_bus_status = BUS_BUSY;
        switch(attr.status)
        {
        case TurnOn:
            setPwm(IR_Status_Pos,attr.level);
            setPwmupdate();
            break;
        case LightingQuick://亮灭各0.5s
            base = base *1;
            if(timingdelaylocal){
                if(timingdelaylocal < base*50U){
                    setPwm(IR_Status_Pos,attr.level);
                    setPwmupdate();
                }else{
                    setPwm(IR_Status_Pos,0);
                    setPwmupdate();
                }
                timingdelaylocal--;
            }else{
                timingdelaylocal = base*100U;
            }
            break;
        case LightingSlow://亮灭各1s
            base = base *2;
            if(timingdelaylocal_slow){
                if(timingdelaylocal_slow < base*50U){
                    setPwm(IR_Status_Pos,attr.level);
                    setPwmupdate();
                }else{
                    setPwm(IR_Status_Pos,0);
                    setPwmupdate();
                }
                timingdelaylocal_slow--;
            }else{
                timingdelaylocal_slow = base*100U;
            }
            break;
        case TurnOff:
            setPwm(IR_Status_Pos,0);
            setPwmupdate();
            break;
        default:
            break;
        }
        i2c_bus_status = BUS_IDLE;
    }
    states = attr.status;
    return 0;
}
#endif