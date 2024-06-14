#include "core_task.h"
#include "systick.h"
//#include "uart.h"

#ifdef GET_WIFI_STATUS_ENABLE
extern volatile u8 global_wifi_status;
#endif

#ifndef MEASURE_SHT_CYCLE
#define MEASURE_SHT_CYCLE (5)
#endif

#ifndef MEASURE_ISFL_CYCLE
#define MEASURE_ISFL_CYCLE (5)
#endif

static shtParam shtp;

extern IIC_BUS_STATUS   i2c_bus_status;
//add
GET_WIFI_STATUS_E mcu_get_wifi_states_e = GET_WIFI_STATUS_IDLE;
//end

//Fixed temperature difference when turning leds on and off
//end   
/**
 * 显示温度    = 测量温度 - 1
   显示湿度    = (测量湿度 *测量温度)/显示温度

上报给app:
显示温度 显示湿度

设备led显示:
小数点 0.3-0.7 显示.5
0.0-0.2 :  不显示.5
0.8-0.9  : 温度 = 温度+1  湿度 = 湿度+1
*/

void sht4x_measure_task(uint8_t *updates,int32_t *temperatures,uint32_t *humiditys){
    int8_t  measure_ret = -1;
    float   temper = 0.00f;
    float   humi = 0.00f;
    static int32_t old_temperature = 0;
    static int32_t old_humidity = 0;
    static uint8_t update = 0;
    static unsigned int  g_record_times = 0;
    float   show_temper = 0.00f;//显示温度
    float   measure_temper = 0.00f;//测量温度
    //湿度 = 湿度*测量温度/show_temper
    float final_tempValue = 0;
    if(get_sht4x_status() == STATUS_OK){
        int32_t temperature, humidity;
        if(i2c_bus_status == BUS_IDLE){
            i2c_bus_status = BUS_BUSY;
            measure_ret = sht4x_measure_detail(&temperature, &humidity);
            i2c_bus_status = BUS_IDLE;
        }
        #if (DEBUG_MODE)
            PR_NOTICE("###sht4x_measure_detail,%d\r\n",measure_ret);
        #endif
        if(measure_ret == STATUS_OK){// 测量正常返回,修正测量数据
            temper = temperature / 1000.0f;
            measure_temper = temper;//add-12-14
            //Fixed temperature difference when turning leds on and off
            float out_temper = temper;
            g_record_times++;
            record_led_status(g_record_times,Get_led_run_mode(),&temper,&out_temper);
            if(g_record_times >= 360){
                g_record_times = 0;
            }
            temper = out_temper;
            //issue:2023-01-14(第二次判定)
            if(temper <= 5 || temper > 35){//-10°~5 &&  >35 (35.1-40°)
                temper = temper - 1.0f;
            }else if((temper > 5 && temper <= 10) || (temper > 30 && temper <= 35)){// 5.1~10° &&  30.1-35°
                temper = temper - 1.1f;
            }else if(temper > 10 && temper <= 30){
                temper = temper - 1.2f;//温差漂移//add:11-26 //add :0.3f 2023-02-03
            }
            //第三次判判定
            if(temper < -9){
                temper = -9;
            }
            if(temper > 49){
                temper = 49;
            }
            //第三次end

            //end:2023-0203
            temper = temper*10;
            humi =  humidity/ 1000.0f;
            //PR_NOTICE("###mcu_dp_value_update,humidity### %0.2f\r\n",humi);
            
            humi = humi*1.055;
            //第一次(判定)修正湿度
            if(humi<1){
                humi = 1.0f;
            }else if(humi > 99){
                humi = 99.0f;
            }

            humi = humi*10;
            temperature = (int)temper;//温度 范围 0- 400 ，其中400 表示40.0  ℃
            humidity = (int)humi;
            //humidity = (round(humidity))/10;//湿度 四舍五入，再除以10

            //如果温湿度测量与上次比较一致，则不更新上报
            if((old_temperature == temperature) && (old_humidity == humidity)){
                update = 0;
            }
            if(old_temperature != temperature){
                old_temperature = temperature;
                update = 1;
            }
            if(old_humidity != humidity){
                old_humidity = humidity;
                update = 1;
            }
            //END
            *updates = update;
            *temperatures = temperature;
            *humiditys = humidity;
        }
    }
}

void update_i2c_show_humi_task(update_method_e method,int32_t temperature,uint32_t humidity){
    switch (method)
    {
    case update_local:
        if(get_IS31FL_status() == IS31FL_OK){
            if(i2c_bus_status == BUS_IDLE){
                i2c_bus_status = BUS_BUSY;
                I2C_CTRL_Show_temp_humi((int16_t)temperature,(uint16_t)humidity);
                i2c_bus_status = BUS_IDLE;
            }
        }
        break;
    case update_cloud:
        #if 0
        if(global_wifi_status == WIFI_CONN_CLOUD){//and wifi已经连上  才上报
            mcu_dp_value_update(DPID_TEMP,temperature); //VALUE型数据上报;
            mcu_dp_value_update(DPID_HUM,humidity); //VALUE型数据上报;
            //printf("###mcu_dp_value_update,ok### %d,,,%d\r\n",temperature,humidity);
        }
        #endif
        break;
    case cloud_local:
        if(get_IS31FL_status() == IS31FL_OK){
            if(i2c_bus_status == BUS_IDLE){
                i2c_bus_status = BUS_BUSY;
                I2C_CTRL_Show_temp_humi((int16_t)temperature,(uint16_t)humidity);
                i2c_bus_status = BUS_IDLE;
            }
        }
        #if 0
        if(global_wifi_status == WIFI_CONN_CLOUD){//and wifi已经连上  才上报
            mcu_dp_value_update(DPID_TEMP,temperature); //VALUE型数据上报;
            mcu_dp_value_update(DPID_HUM,humidity); //VALUE型数据上报;
            //PR_NOTICE("###mcu_dp_value_update,ok### %d\r\n",temperature);
        }
        #endif
        break;
    default:
        break;
    }
}

void setShtParams(int32_t temperature,uint32_t humidity){
    shtp.humidity = humidity;
    shtp.temperature = temperature;
}

void measure_task_exe(){
    #define TEMP_CONVERT_FLAG 0 //温度标识转换
    static uint8_t  updates= 0;
    static int32_t  temperatures;
    static uint32_t humidity;
    uint16_t temper;
    uint8_t update_completed = 0;//是否完成本次更新
    float   temper_value;
    //test
    #if DO_LOCAL_TEST_TEMP
    static uint8_t n_temperatures = 1;
    static uint8_t n_humidity = 1;
    #endif
    #define reporting_freq 1
    #if reporting_freq //湿度与温度上报频率;更新:20240508
    static int32_t  temperatures_clouds = 0;
    static uint32_t humidity_clouds = 0;
    static uint32_t update_times_freq = 0;//1分钟20==> 20*15
    #endif

    //每隔 MEASURE_SHT_CYCLE 测量一次当前环境温湿度
    if(GetTimeTaskParams(sht4x_device)!= 0 && (GetTimeTaskParams(sht4x_device)%MEASURE_SHT_CYCLE == 0)){
        resetTimeTaskParams(sht4x_device);
        //开机大约 MEASURE_SHT_CYCLE,秒数开始测量温度
        ConfigTimerTask(0);
        sht4x_measure_task(&updates,&temperatures,&humidity);//测量+修正+偏差+限值

        #if reporting_freq //湿度与温度上报频率;更新:20240508 version:1.4.8
        if(update_times_freq == 5*12){
            temperatures_clouds = temperatures;
            humidity_clouds = humidity;
            update_times_freq = 0;
            update_completed = 1;
        }
        //当温度差0.3,或者湿度差3%时，置标志位update_completed上报到涂鸦云
        if((temperatures - temperatures_clouds > 3) || (humidity - humidity_clouds > 30)|| (temperatures_clouds - temperatures > 3)){
            temperatures_clouds = temperatures;
            humidity_clouds = humidity;
            update_times_freq = 0;
            update_completed = 1;
        }else{
            update_times_freq++;
        }
        if(update_completed){
            update_i2c_show_humi_task(update_cloud,temperatures_clouds,humidity_clouds);
        }
        #else
            update_i2c_show_humi_task(update_cloud,temperatures,humidity);//云端上报
        #endif

        //update_i2c_show_humi_task(update_cloud,temperatures,humidity);//云端上报

        ConfigTimerTask(1);
        setShtParams(temperatures,humidity);//参数更新
        //printf("temperatures:%d,humidity:%d\r\n",temperatures,humidity);
    }
    //每隔 MEASURE_ISFL_CYCLE 更新LED 以及上报到云端
    if(GetTimeTaskParams(is31fl_device)!= 0 && (GetTimeTaskParams(is31fl_device)%MEASURE_ISFL_CYCLE == 0)){
        resetTimeTaskParams(is31fl_device);
        if(updates&& update_completed)
        {
            led_dispaly_state(Degree_Symbol_Centigrade_Part,LED_SHUT_DOWN);
            led_dispaly_state(Degree_Symbol_Fahrenheit_Part,LED_SHUT_DOWN);
            #if TEMP_CONVERT_FLAG
            if(GetCurrentTempConverStatus() == 1){
                temper_value = (float)temperatures/10;//实际温度值应除以10
                temper_value = (temper_value*1.8)+32;//摄氏度与华氏度转换公式
                temperatures = (uint32_t)(temper_value*10);
                led_dispaly_state(Degree_Symbol_Fahrenheit_Part,LED_NORMAL);
            }else{
                led_dispaly_state(Degree_Symbol_Centigrade_Part,LED_NORMAL);
            }
            #endif
            update_i2c_show_humi_task(update_local,temperatures,humidity);//本地led(0.5偏差显示,含华氏度99以上只显示99) 
        }
        #if DO_LOCAL_TEST_TEMP
            update_i2c_show_humi_task(update_local,n_temperatures*110,n_humidity*90);
            n_temperatures++;
            n_humidity++;
            if(n_temperatures >= 9){
                n_temperatures=0;
                n_humidity = 0;
            }
        #endif
    }
}

//接收到温标改变的指令
void update_convert_tempflag(uint8_t flag){
    led_dispaly_state(Degree_Symbol_Centigrade_Part,LED_SHUT_DOWN);
    led_dispaly_state(Degree_Symbol_Fahrenheit_Part,LED_SHUT_DOWN);
    //更新温标℉ <-> ℃
    float    temper_value = 0;
    uint32_t temperatures = 0;
    uint32_t humidity = shtp.humidity;
    // ℉ = ℃ * 1.8000 + 32.00  
    if(flag == 1){
        temper_value = (float)shtp.temperature/10;//实际温度值应除以10
        temper_value = (temper_value*1.8)+32;//摄氏度与华氏度转换公式
        temperatures = (uint32_t)(temper_value*10);
        led_dispaly_state(Degree_Symbol_Fahrenheit_Part,LED_NORMAL);
    }else{
        temperatures = shtp.temperature;
        led_dispaly_state(Degree_Symbol_Centigrade_Part,LED_NORMAL);
    }
    update_i2c_show_humi_task(update_local,temperatures,humidity);
}

#define WIFI_INDICATOR_STATE 0x07 //网络异常

void wifi_status_time_task()
{
    #if 0
    static uint8_t up_times = 3;
    uint8_t led_level = get_current_led_level();
    if(mcu_get_wifi_states_e == GET_WIFI_STATUS_IDLE){
        mcu_get_wifi_states_e = GET_WIFI_STATUS_BUSY;
        mcu_get_rssi_cmd_status();//issuse ble with wifi status(0112)
        mcu_get_wifi_connect_status();
        mcu_get_wifi_states_e = GET_WIFI_STATUS_IDLE;
    }
    //add-增加上电检测wifi状态,若没配网,则主动重置一次(10-12)
    if(global_wifi_status == SMART_CONFIG_STATE || global_wifi_status == WIFI_LOW_POWER){
        #ifdef FIRST_POWER_WIFI_STAUTS_DET
        if(GetTimeTaskParams(wifi_device) < FIRST_POWER_WIFI_STAUTS_DET)
        {
            mcu_reset_wifi();
            resetTimeTaskParams(wifi_device);
        }
        #endif
    }
    //PR_NOTICE("###wifi_status###:%d \r\n",global_wifi_status);
    if(get_IS31FL_status() == IS31FL_OK){
        if(global_wifi_status == WIFI_CONN_CLOUD && GetRssiCmdData() == 0){//issuse ble with wifi status(0112)
            set_wifiled_status(TurnOff,led_level);
            return;
        }
        switch (global_wifi_status)
        {
            case SMART_CONFIG_STATE:
                set_wifiled_status(LightingQuick,led_level);
                break;
            case AP_STATE:
                set_wifiled_status(LightingSlow,led_level);
                up_times = 2;
                break;
            case WIFI_NOT_CONNECTED:
                set_wifiled_status(TurnOff,led_level);
                led_dispaly_state(Net_Status_Indicator,LED_NORMAL);
                break;
            case WIFI_CONNECTED:
                set_wifiled_status(TurnOff,led_level);
                led_dispaly_state(Net_Status_Indicator,LED_NORMAL);
                break;
            case WIFI_CONN_CLOUD:
                led_dispaly_state(Net_Status_Indicator,LED_SHUT_DOWN);
                set_wifiled_status(TurnOn,led_level);
                break;
            case WIFI_LOW_POWER:
                set_wifiled_status(TurnOff,led_level);
                led_dispaly_state(Net_Status_Indicator,LED_NORMAL);
                break;
            case SMART_AND_AP_STATE:
                led_dispaly_state(Net_Status_Indicator,LED_SHUT_DOWN);
                set_wifiled_status(LightingQuick,led_level);
                up_times = 2;
                break;
            default:
                break;
        }
        #if (DEBUG_MODE)
        PR_NOTICE("set_wifiled_status,ok \r\n");
        #endif
    }
    if(global_wifi_status == WIFI_CONN_CLOUD && GetRssiCmdData() != 0){//issuse ble with wifi status(0112)
        if(up_times > 0){
            update_remote_todo();
            up_times--;
        }
    }
    #endif
}

#if 0
void check_wifi_indicator(){
    //大约10s统计并计算,wifi状态以3s状态更新,
    //网络不良或已断网,更新最慢时间为半分钟(30s),一般时间为20-30s
    #ifdef WIFI_INDICATOR_STAUTS_DET
    if(Getwifi_indicator_cycle() >= WIFI_INDICATOR_STAUTS_DET){
        if(s_not_connect_times >= 2){
            global_wifi_status = WIFI_INDICATOR_STATE;
        }
        if(s_connect_times >= 2){
            global_wifi_status = WIFI_INDICATOR_STATE;//异常
        }
        configwifi_status_time_cycle(Reset_Wifi_indicator_cycle);//重新计数
        //清空记录的次数(在规定时间内网络不良的)
        s_not_connect_times = 0;
        s_connect_times = 0;
        s_connect_cloud_times = 0;
    }
    #endif
}
#endif