#ifndef USEBASE_TIMER_H
#define USEBASE_TIMER_H

#include <stdint.h>

//add
#ifndef FIRST_POWER_WIFI_STAUTS_DET
#define FIRST_POWER_WIFI_STAUTS_DET 15
#endif 

#ifndef WIFI_INDICATOR_STAUTS_DET
#define WIFI_INDICATOR_STAUTS_DET 9
#endif

typedef enum
{
    sht4x_device,
    is31fl_device,
    wifi_device,
}time_task_device_e;

typedef enum
{
    Reset_Wifi_timer_cycle,
    Set_Wifi_timer_cycle,
    Reset_Wifi_indicator_cycle,
}wifi_status_time_cycle_e;

int  useGeneralTimerProcess(int number,uint16_t t_ms);
void    ConfigTimerTask(uint8_t taskOn);
uint8_t GetTimerTaskFlag();
uint8_t Getwifi_status_time_cycle();
uint8_t Getwifi_indicator_cycle();
void    configwifi_status_time_cycle(wifi_status_time_cycle_e e);
void    resetTimeTaskParams(time_task_device_e e);
uint8_t  GetTimeTaskParams(time_task_device_e e);
//add
void    record_led_status(unsigned int rec_time,unsigned char on_off,float* value,float *out_value);
#endif