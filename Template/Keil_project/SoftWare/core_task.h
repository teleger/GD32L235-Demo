#ifndef __CORE_TASK_H
#define __CORE_TASK_H


#ifdef GD32L235
#include "gd32l23x.h"
#endif
#ifdef GD32E230
#include "gd32e23x.h"
#endif
#include <stdio.h>
#include "useBaseTime.h"
#include "sht4x_usage_demo.h"
//#include "wifi.h"
#include "i2c_led.h"
//#include "button.h"
//状态
typedef enum{
    GET_WIFI_STATUS_IDLE,//空闲
    GET_WIFI_STATUS_BUSY
}GET_WIFI_STATUS_E;

typedef enum {
    update_local,
    update_cloud,
    cloud_local,
}update_method_e;

typedef struct _shtParam
{
    volatile int32_t temperature;
    volatile uint32_t humidity;
}shtParam;

void sht4x_measure_task(uint8_t *updates,int32_t *temperatures,uint32_t *humiditys);
void update_i2c_show_humi_task(update_method_e method,int32_t temperature,uint32_t humidity);
void wifi_status_time_task();
void measure_task_exe();
void setShtParams(int32_t temperature,uint32_t humidity);
void update_convert_tempflag(uint8_t flag);

#endif