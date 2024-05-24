#ifndef I2C_LED_H
#define I2C_LED_H

#include <stdint.h>
#include "common_i2c_attr.h"
typedef enum{
    TurnOn=0,//常亮
    LightingQuick,//快速闪烁
    LightingSlow,//缓慢闪烁
    TurnOff//关闭
}LedStatus;

typedef struct _I2C_LED_ATTR_
{/* data */
    uint8_t     level;
    LedStatus   status;
}I2C_LED_ATTR_S;

enum{
    IS31FL_UNKOWN=0xff,
    IS31FL_OK=0,
    IS31FL_FAIL=1,
};

typedef enum _LED_RUN_MODE{
    LED_NORMAL,
    LED_SHUT_DOWN
}LED_RUN_MODE;

//add 04-29 增加红外指示灯
typedef enum{
    Temp_Hum_Percent=0x1d,
    Wifi_Channle_Pos_Value,
    Net_Status_Indicator,//网络异常指示灯
    Degree_Symbol_Centigrade_Part,//摄氏度
    Degree_Symbol_Fahrenheit_Part,//华氏度
    Temp_Decimal_Point,
    Humi_Decimal_Point,
    IR_Status_Pos
}Led_Point_Channle_Map;

//属性attribute
uint8_t led_wifi_status(I2C_LED_ATTR_S attr);
void    set_wifiled_status(LedStatus status,uint8_t levels);
void    init_led_status();

uint8_t     IS31_WriteOneByte_timeout(uint8_t WriteAddr,uint8_t DataToWrite);
void        IS31FL3236_Init();
uint8_t     IS31_WriteOneByte_buffer_timeout(uint8_t WriteStartAddr,uint8_t WriteEndAddr,uint8_t DataToWrite);
uint8_t     get_IS31FL_status(void);

void        setRunMode(LED_RUN_MODE mode);
LED_RUN_MODE Get_led_run_mode();
#ifdef IIC_LED_TEST_DEMO
int     i2cled_test_process();
#endif

//sht4x interface api 
void I2C_CTRL_Show_temp_humi(int16_t temperature,uint16_t humidity);
void update_data_prepare_number();
void led_show_number(uint8_t number);
void led_dispaly_state(Led_Point_Channle_Map m,LED_RUN_MODE mode);
void set_led_level(uint8_t level);
uint8_t get_current_led_level();
void IIC2_All_Init();
//add
void showIR_status(uint8_t ir_status);
void show_negative(uint8_t symbol);
//uint8_t led_remote_status(I2C_LED_ATTR_S attr);
#endif 