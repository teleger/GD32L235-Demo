/*!

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


#include "gd32l23x.h"
#include "gd32l23x_timer.h"
#include "gd32l23x_rcu.h"

#ifdef GD32E230
#include "gd32e23x.h"
#endif

#include <stdio.h>
#include "useBaseTime.h"
#include "sht4x_usage_demo.h"
//#include "wifi.h"
#include "i2c_led.h"
//#include "button.h"
#include "main.h"
//add UseTimer

typedef enum {
    useTIMER11 = TIMER11,
    useTIMER14 = TIMER14,
    useTIMER40 = TIMER40
}UseTimer_Map;

typedef enum {
    useTIMER11_IRQn = TIMER11_IRQn,
    useTIMER14_IRQn = TIMER14_IRQn,
    useTIMER40_IRQn = TIMER40_IRQn,
    useTIMER16_IRQn
}UseTimer_IRQn_Map;

// typedef enum {
//     useRCU_TIMER13 = RCU_TIMER13,
//     useRCU_TIMER14 = RCU_TIMER14,
//     useRCU_TIMER15 = RCU_TIMER15,
//     useRCU_TIMER16 = RCU_TIMER16
// }UseRCU_TIMER_Map;

/* configure the TIMER peripheral */
// static void timer_config(UseTimer_Map timer_n,rcu_periph_enum periph,uint32_t periods);
// /* configure the TIMER interrupt */
// static void nvic_config(UseTimer_IRQn_Map timer_IRQn);

/**
    \brief      configure the TIMER interrupt
    \param[in]  none
    \param[out] none
    \retval     none
  */
static void nvic_config(UseTimer_IRQn_Map timer_IRQn)
{
    nvic_irq_enable(timer_IRQn, 1);
}

/**
    \brief      configure the TIMER peripheral
    \param[in]  none
    \param[out] none
    \retval     none
  */
static void timer_config(UseTimer_Map timer_n,rcu_periph_enum periph,uint32_t periods)
{
    /* ----------------------------------------------------------------------------
    TIMER2 Configuration: 
    TIMER2CLK = SystemCoreClock/7200 = 10KHz,the period is 1s( period_x/10000 = 1s).
    //if period == 10000,so ...  1S
    //if period == 100,so ... 100/10000 = 0.01 s  = 10ms
    ---------------------------------------------------------------------------- */
    timer_oc_parameter_struct timer_ocinitpara;
    timer_parameter_struct timer_initpara;

    /* enable the peripherals clock */
    rcu_periph_clock_enable(periph);

    /* deinit a TIMER */
    timer_deinit(timer_n);
    /* initialize TIMER init parameter struct */
    timer_struct_para_init(&timer_initpara);
    /* TIMER2 configuration */
    if(8000000 == rcu_clock_freq_get(CK_SYS)){
        timer_initpara.prescaler         = 799;//if is 8MHz
    }else{
        timer_initpara.prescaler         = 6399;//if is 64MHz
    }
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = (periods-1);
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_init(timer_n, &timer_initpara);

    /* clear channel 0 interrupt bit */
    timer_interrupt_flag_clear(timer_n, TIMER_INT_FLAG_UP);
    /* enable the TIMER interrupt */
    timer_interrupt_enable(timer_n, TIMER_INT_UP);
    /* enable a TIMER */
    timer_enable(timer_n);
}
static volatile uint8_t timerTaskFlag = 1;
//add
volatile uint8_t  wifi_status_time_cycle = 0;
volatile uint8_t  wifi_indicator_cycle = 0;
//定时器任务参数结构
typedef struct _TimeTaskParams
{
    volatile uint8_t first_power_wifi_status;//add
    volatile uint8_t Timer13_sht4x_Value;
    volatile uint8_t Timer13_is31fl_value; 
}TimeTaskParams;

static TimeTaskParams timetaskparams={0,0,0};
extern I2C_LED_ATTR_S local_i2c_led_value;

#define UseTimer11 
#define UseTimer14
#define UseTimer40
/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int useGeneralTimerProcess(const int number,uint16_t t_ms)
{
    switch (number)
    {
    case 11:
        // /* configure the TIMER peripheral */
        timer_config(TIMER11,RCU_TIMER11,t_ms);
        nvic_config(useTIMER11_IRQn);
        break;
    case 14:
        // /* configure the TIMER peripheral */
        timer_config(TIMER11,RCU_TIMER11,t_ms);
        nvic_config(useTIMER14_IRQn);
        break;
    case 40:
        // /* configure the TIMER peripheral */
        timer_config(TIMER40,RCU_TIMER40,t_ms);
        nvic_config(useTIMER40_IRQn);
        break;
    default:
        break;
    }
    return 0;
}

//add增加(修正在关闭led的情况下温度的偏差),date:2023-02-03
//Fixed temperature difference when turning leds on and off
//add date: 2023-08-26,version:1.4.2
void record_led_status(unsigned int rec_time,unsigned char on_off,float* value,float *out_value){
    static unsigned char on_times = 0;//led 开灯持续,24min，0.6°
    static unsigned char off_times = 0;//led 灯次数,12min，0.6°
    static unsigned char base = 0;//limit base (0 ~ 6 之间)
    static float old_value = 99.9f;
    if(on_off){//is 1,led shutdown 
        off_times += 1;//当前led关,记录+1
    }else{
        on_times  += 1;//当前led开,记录+1
    }
    if(value == NULL)return;
    float tmp_value = *value;
    if(tmp_value == 0)return;
    if(rec_time != 0 && rec_time%20 == 0){
        {//在1min内统计 
            if(on_times >= 15){//1min内达到50,真实有效            50 ==>1
                if(base > 0 && base <= 6){//0-6
                    base -= 1;
                }
            }
            if(off_times >= 15){
                if(base >= 0 && base < 6){
                    base += 1;
                }
            }
            tmp_value = tmp_value + 0.1*base;
            old_value = tmp_value;
            on_times = 0;
            off_times = 0;//清空
        }
    }
    if(base > 0){
        *out_value = old_value;
        return;    
    }
    *out_value = tmp_value;
}

static void time_cycle(){
    static __IO uint8_t Timer_Count_Value = 0;
    Timer_Count_Value++;
    if((Timer_Count_Value == 2)|| ((Timer_Count_Value>=2)&&(Timer_Count_Value%2==0)))
    {//500ms间隔，最低设置为1S
        if(get_sht4x_status() == STATUS_OK){
            timetaskparams.Timer13_sht4x_Value++;
        }
        if(get_IS31FL_status() == IS31FL_OK){
            timetaskparams.Timer13_is31fl_value++;
        }
        #if 0
        Timer13_Remotec_Value++;
        if(GetTimer13_Remotec_Value() == 20)
        {
            resetTimer13_Remotec_Value();
        }
        #endif
        if(timetaskparams.first_power_wifi_status < FIRST_POWER_WIFI_STAUTS_DET){
            timetaskparams.first_power_wifi_status++;
        }
        wifi_indicator_cycle++;
    }
    if((Timer_Count_Value == 3)||((Timer_Count_Value>=3)&&(Timer_Count_Value%3==0)) ){//最低设置为3S
        wifi_status_time_cycle++;
    }
    if(Timer_Count_Value >= 60){
        Timer_Count_Value = 0;
    }
}

#ifdef UseTimer11
void TIMER11_IRQHandler(void){
    if(SET == timer_interrupt_flag_get(TIMER11, TIMER_INT_FLAG_UP)){
        /* clear channel 0 interrupt bit */
        timer_interrupt_flag_clear(TIMER11, TIMER_INT_FLAG_UP);
        time_cycle();
    }
}
#endif

#ifdef UseTimer14
void TIMER14_IRQHandler(void){
    #if 0
    if(SET == timer_interrupt_flag_get(TIMER15, TIMER_INT_FLAG_UP)){
        /* clear channel 0 interrupt bit */
        timer_interrupt_flag_clear(TIMER15, TIMER_INT_FLAG_UP);
        led_wifi_status(local_i2c_led_value);
    }
    #endif
}
#endif


#ifdef UseTimer40
void TIMER40_IRQHandler(void){
    if(SET == timer_interrupt_flag_get(TIMER40, TIMER_INT_FLAG_UP)){
        /* clear channel 0 interrupt bit */
        timer_interrupt_flag_clear(TIMER40, TIMER_INT_FLAG_UP);
        //Button_Process();
    }
}
#endif

//关闭定时器，减少中断
void ConfigTimerTask(uint8_t taskOn){
    if(taskOn){
        timer_enable(TIMER11);
        timer_enable(TIMER14);
        timerTaskFlag = 1;
    }else{
        timer_disable(TIMER11);
        timer_disable(TIMER14);
        timerTaskFlag = 0;
    }
}

uint8_t GetTimerTaskFlag(){
    return timerTaskFlag;
}

uint8_t Getwifi_status_time_cycle(){
    return wifi_status_time_cycle;
}

uint8_t Getwifi_indicator_cycle(){
    return wifi_indicator_cycle;
}

void configwifi_status_time_cycle(wifi_status_time_cycle_e e){
    switch (e)
    {
    case Reset_Wifi_timer_cycle:
        wifi_status_time_cycle = 0;
        break;
    case Set_Wifi_timer_cycle:
        wifi_status_time_cycle = 1;
        break;
    case Reset_Wifi_indicator_cycle:
        wifi_indicator_cycle = 0;
        break;
    default:
        break;
    }
}

uint8_t  GetTimeTaskParams(time_task_device_e e){
    uint8_t value = 0;
    switch (e)
    {
    case sht4x_device:
        value = timetaskparams.Timer13_sht4x_Value;
        break;
    case is31fl_device:
        value = timetaskparams.Timer13_is31fl_value; 
        break;
    case wifi_device:
        value = timetaskparams.first_power_wifi_status; 
        break;
    default:
        break;
    }
    return value;
}

void  resetTimeTaskParams(time_task_device_e e){
    switch (e)
    {
    case wifi_device:
        timetaskparams.first_power_wifi_status = 0xff;
        break;
    case sht4x_device:
        timetaskparams.Timer13_sht4x_Value = 0;
        break;
    case is31fl_device:
        timetaskparams.Timer13_is31fl_value = 0; 
        break;
    default:
        break;
    }
}
