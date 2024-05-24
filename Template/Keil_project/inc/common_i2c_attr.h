#ifndef COMMON_I2C_ATTR_H
#define COMMON_I2C_ATTR_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//add: some timeout struct and constant 
#ifdef GD32E230
typedef enum
{
    I2C_START = 0,
    I2C_SEND_ADDRESS,
    I2C_CLEAR_ADDRESS_FLAG,
    I2C_TRANSMIT_DATA,
    I2C_STOP
}i2c_process_enum;
#endif
#ifdef GD32L235
typedef enum {
    I2C_START = 0,
    I2C_SEND_ADDRESS,
    I2C_RESTART,
    I2C_TRANSMIT_DATA,
    I2C_RELOAD,
    I2C_STOP,
    I2C_END
} i2c_process_enum;
#endif
//IIC总线状态
typedef enum{
    BUS_IDLE,//空闲
    BUS_BUSY
}IIC_BUS_STATUS;

#define I2C_TIME_OUT   (uint16_t)(2000)
#define I2C_OK         1
#define I2C_FAIL       0
#define I2C_END        1
#define I2CX           I2C0

void i2c_bus_reset();
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMMON_I2C_ATTR_H */