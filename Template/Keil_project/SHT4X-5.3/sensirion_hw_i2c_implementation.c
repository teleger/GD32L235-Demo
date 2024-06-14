/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "sensirion_arch_config.h"
#include "sensirion_common.h"
#include "sensirion_i2c.h"
#include "common_i2c_attr.h"

#ifdef GD32E230
#include "gd32e23x.h"
#endif

#ifdef GD32L235
#include "gd32l23x.h"
#endif

#include "systick.h"
#define SENSIR_STR "sensirion"
/*
 * INSTRUCTIONS
 * ============
 *
 * Implement all functions where they are marked as IMPLEMENT.
 * Follow the function specification in the comments.
 */

/**
 * Select the current i2c bus by index.
 * All following i2c operations will be directed at that bus.
 *
 * THE IMPLEMENTATION IS OPTIONAL ON SINGLE-BUS SETUPS (all sensors on the same
 * bus)
 *
 * @param bus_idx   Bus index to select
 * @returns         0 on success, an error code otherwise
 */
int16_t sensirion_i2c_select_bus(uint8_t bus_idx) {
    // IMPLEMENT or leave empty if all sensors are located on one single bus
    return STATUS_FAIL;
}

/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication.
 */
void sensirion_i2c_init(void) {
    // IMPLEMENT
    sensirion_i2c_rcu_config();
    sensirion_i2c_gpio_config();
    sensirion_i2c_attribute_config();
}

/**
 * Release all resources initialized by sensirion_i2c_init().
 */
void sensirion_i2c_release(void) {
    // IMPLEMENT or leave empty if no resources need to be freed
}



/**
 * Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to read from
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in the buffer
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_read(uint8_t address, uint8_t* data, uint16_t count) {
    // IMPLEMENT
    uint8_t ret = 0xff;
    ret = sensirion_i2c_ReadByte_timeout(address,data,count);
    //printf("sensirion_i2c_read:%d\r\n",ret);
    if(ret != I2C_END)return STATUS_FAIL;
    return NO_ERROR;
}

/**
 * Execute one write transaction on the I2C bus, sending a given number of
 * bytes. The bytes in the supplied buffer must be sent to the given address. If
 * the slave device does not acknowledge any of the bytes, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to write to
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from the buffer and send over I2C
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_write(uint8_t address, const uint8_t* data,
                           uint16_t count) {
    // IMPLEMENT
    uint8_t ret = 0xff;
    ret = sensirion_i2c_WriteByte_timeout(address,data,count);
    //printf("sensirion_i2c_hal_write,address:0x%x,data:0x%x,count:%d \r\n",address,*data,count);
    if(ret != I2C_END ){//i2c_WriteByte Fail
        printf("i2c_WriteByte Fail:%d\r\n",ret);
        return STATUS_FAIL;
    }
    return NO_ERROR;
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * Despite the unit, a <10 millisecond precision is sufficient.
 *
 * @param useconds the sleep time in microseconds
 */
void sensirion_sleep_usec(uint32_t useconds) {
    // IMPLEMENT
    if(useconds % 1000 == 0){
        uint32_t mseconds = useconds /1000;
        delay_1ms(mseconds);
    }
}

//add
#ifndef I2C1_OWN_ADDRESS7 
#define I2C1_OWN_ADDRESS7      0x82
#endif

//add-sensirion-some-api
//(including init. config i2c pin,clk, ....  send one byte and so on)
void sensirion_i2c_WriteOneByte(uint8_t Addr,uint8_t data){
    #ifdef GD32E230
    /* wait until I2C bus is idle */
    while(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY));
    //printf("sensirion_i2c_WriteOneByte,#1,data:%d \r\n ",data);
    i2c_start_on_bus(I2C1);//i2c_start
    
    while(!i2c_flag_get(I2C1, I2C_FLAG_SBSEND));
    //printf("sensirion_i2c_WriteOneByte,#11,address:0x%x \r\n ",Addr);

    i2c_master_addressing(I2C1,(Addr << 1),I2C_TRANSMITTER);
    //printf("i2c_master_addressing,ok\r\n ");
    
    while(!i2c_flag_get(I2C1, I2C_FLAG_ADDSEND));
    //printf("sensirion_i2c_WriteOneByte,#2 \r\n ");
    i2c_flag_clear(I2C1, I2C_FLAG_ADDSEND);
    
    while(!i2c_flag_get(I2C1, I2C_FLAG_TBE));
    //printf("sensirion_i2c_WriteOneByte,#3 \r\n ");

    i2c_data_transmit(I2C1,data);//data
    while(!i2c_flag_get(I2C1, I2C_FLAG_BTC));
    i2c_stop_on_bus(I2C1);
    
    while(I2C_CTL1(I2C1)&0x0200);
    #endif
    //delay_1ms(10);
    //PR_DEBUG("#sensirion_i2c_WriteOneByte#,ok \r\n");
}

#if 0
void sensirion_i2c_WriteByte(uint8_t Addr,const uint8_t *data,uint16_t count){
    /* wait until I2C bus is idle */
    while(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY));
    //printf("sensirion_i2c_WriteByte,#1,data:%d \r\n ",data);
    i2c_start_on_bus(I2C1);//i2c_start
    
    while(!i2c_flag_get(I2C1, I2C_FLAG_SBSEND));
    //printf("sensirion_i2c_WriteByte,#11,address:0x%x \r\n ",Addr);

    i2c_master_addressing(I2C1,(Addr << 1),I2C_TRANSMITTER);
    //printf("i2c_master_addressing,ok\r\n ");
    
    while(!i2c_flag_get(I2C1, I2C_FLAG_ADDSEND));
    //printf("sensirion_i2c_WriteByte,#2 \r\n ");
    i2c_flag_clear(I2C1, I2C_FLAG_ADDSEND);
    
    while(!i2c_flag_get(I2C1, I2C_FLAG_TBE));
    //printf("sensirion_i2c_WriteByte,#3 \r\n ");

    while(count){
        i2c_data_transmit(I2C1,*data);//data
        data++;
        count--;
        while(!i2c_flag_get(I2C1, I2C_FLAG_BTC));
    }
    
    i2c_stop_on_bus(I2C1);
    while(I2C_CTL1(I2C1)&0x0200);

    PR_DEBUG("#sensirion_i2c_WriteByte#,ok \r\n");
}

void sensirion_i2c_ReadByte(uint8_t Addr,uint8_t *data,uint16_t count){
    i2c_ackpos_config(I2C1, I2C_ACKPOS_NEXT);

    /* wait until I2C bus is idle */
    while(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY));
    //PR_DEBUG("sensirion_i2c_ReadOneByte,#1,data:%d \r\n",data);
    i2c_start_on_bus(I2C1);//i2c_start
    
    while(!i2c_flag_get(I2C1, I2C_FLAG_SBSEND));
    //PR_DEBUG("sensirion_i2c_ReadOneByte,#11,address:0x%x \r\n ",Addr);

    i2c_master_addressing(I2C1,(Addr << 1),I2C_RECEIVER);
    //PR_DEBUG("i2c_master_addressing,ok\r\n ");

    /* disable ACK before clearing ADDSEND bit */
    //i2c_ack_config(I2C1, I2C_ACK_DISABLE);

    while(!i2c_flag_get(I2C1, I2C_FLAG_ADDSEND));
    //PR_DEBUG("sensirion_i2c_ReadOneByte,#2 \r\n ");
    i2c_flag_clear(I2C1, I2C_FLAG_ADDSEND);

    #if (1)
    // /* Wait until the last data byte is received into the shift register */
    //while(!i2c_flag_get(I2C1, I2C_FLAG_BTC));
    //PR_DEBUG("i2c_flag_get,I2C_FLAG_BTC,ok,ok,#22 \r\n ");
    /* wait until the RBNE bit is set */
    while(count){
        if(count == 1){
            i2c_ackpos_config(I2C1, I2C_ACKPOS_CURRENT);
            i2c_ack_config(I2C1, I2C_ACK_DISABLE);
        }
        if(i2c_flag_get(I2C1, I2C_FLAG_RBNE)){
            *data = i2c_data_receive(I2C1);
            i2c_ack_config(I2C1, I2C_ACK_ENABLE);
            data++;
            count--;
        }
    }
    PR_DEBUG("ReadByte\r\n ");
    #else
        for(i=0; i<3; i++){
            if(0 == i){
                /* wait until the second last data byte is received into the shift register */
                while(!i2c_flag_get(I2C1, I2C_FLAG_BTC));
                /* disable acknowledge */
                i2c_ack_config(I2C1, I2C_ACK_DISABLE);
            }
            /* wait until the RBNE bit is set */
            while(!i2c_flag_get(I2C1, I2C_FLAG_RBNE));
            /* read a data from I2C_DATA */
            data1 = i2c_data_receive(I2C1);
            PR_DEBUG("i2c_data_receive,ok,ok::%d #22 \r\n ",data1);
        }
    #endif
    
    /* send a stop condition */
    i2c_stop_on_bus(I2C1);
    while(I2C_CTL1(I2C1)&0x0200);
    i2c_ackpos_config(I2C1, I2C_ACKPOS_CURRENT);
    /* enable acknowledge */
    i2c_ack_config(I2C1, I2C_ACK_ENABLE);

    PR_DEBUG("sensirion_i2c_ReadByte,end \r\n ");
}
#endif

//add
uint8_t sensirion_i2c_WriteByte_timeout(uint8_t Addr,const uint8_t *data,uint16_t count){
    #ifdef  GD32E230
    uint8_t     state = I2C_START;
    uint16_t    timeout = 0;
    uint8_t     i2c_timeout_flag = 0;
    while(!(i2c_timeout_flag)){
        switch (state)
        {
        case I2C_START:
            while(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            if(timeout < I2C_TIME_OUT){
                i2c_start_on_bus(I2C1);//i2c_start
                timeout = 0;
                state = I2C_SEND_ADDRESS;
            }else{
                timeout = 0;
                state = I2C_START;
                i2c_timeout_flag = I2C_OK;
                //PR_ERR("%s,i2c bus is busy in byte write \r\n",SENSIR_STR);
            }
            break;
        case I2C_SEND_ADDRESS:
            /* i2c master sends START signal successfully */
            while((!i2c_flag_get(I2CX, I2C_FLAG_SBSEND)) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            if(timeout < I2C_TIME_OUT){
                i2c_master_addressing(I2CX, (Addr << 1), I2C_TRANSMITTER);
                timeout = 0;
                state = I2C_CLEAR_ADDRESS_FLAG;
            }else{
                timeout = 0;
                state = I2C_START;
                i2c_timeout_flag = I2C_OK;
                //PR_ERR("%s,i2c master sends start signal timeout in BYTE WRITE!\r\n",SENSIR_STR);
            }
            break;
        case I2C_CLEAR_ADDRESS_FLAG:
            /* address flag set means i2c slave sends ACK */
            while((!i2c_flag_get(I2CX, I2C_FLAG_ADDSEND)) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            if(timeout < I2C_TIME_OUT){
                i2c_flag_clear(I2CX, I2C_FLAG_ADDSEND);
                timeout = 0;
                state = I2C_TRANSMIT_DATA;
            }else{
                timeout = 0;
                state = I2C_START;
                //add 提前退出循环
                i2c_timeout_flag = I2C_OK;
                //PR_ERR("%s,i2c master clears address flag timeout in BYTE WRITE!\r\n",SENSIR_STR);
            }
            break;
        case I2C_TRANSMIT_DATA:
            /* wait until the transmit data buffer is empty */
            while((!i2c_flag_get(I2CX, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }

            while(count)
            {
                i2c_data_transmit(I2CX, *data);
                data++;
                count--;
                while((!i2c_flag_get(I2CX, I2C_FLAG_BTC)) && (timeout < I2C_TIME_OUT)){
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT){
                    timeout = 0;
                }else{
                    timeout = 0;
                    state = I2C_START;
                    i2c_timeout_flag = I2C_OK;
                    //PR_ERR("%s,i2c master sends data timeout in PAGE WRITE!\n",SENSIR_STR);
                    break;
                }
            }
            state = I2C_STOP;
            timeout = 0;
            break;
        case I2C_STOP:
            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(I2CX);
            /* i2c master sends STOP signal successfully */
            while((I2C_CTL1(I2CX) & 0x0200) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            if(timeout < I2C_TIME_OUT){
                timeout = 0;
                state = I2C_END;
                i2c_timeout_flag = I2C_OK;
            }else{
                timeout = 0;
                state = I2C_START;
                //PR_ERR("%s,i2c master sends stop signal timeout in BYTE WRITE\r\n",SENSIR_STR);
            }
            break;
        default:
            state = I2C_START;
            i2c_timeout_flag = I2C_OK;
            timeout = 0;
            //PR_DEBUG("%s,i2c master sends start signal in BYTE WRITE!\r\n",SENSIR_STR);
            break;
        }
    }
    #endif

    #ifdef GD32L235
    i2c_process_enum        state = I2C_START;
    uint16_t                timeout = 0;
    uint8_t                 end_flag = 0;
    i2c_transfer_byte_number_config(I2C0, 16);
    //printf("i2c_master_addressing:0x%x\r\n",Addr<<1);
    while(!(end_flag)){
        //printf("IIC writeByte,state: %d\r\n",state);
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
                    //printf("i2c bus is busy in  writeByte! %d\r\n",__LINE__);
                }
                break;
            case I2C_SEND_ADDRESS:
                while((!i2c_flag_get(I2C0, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)){
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT){
                    timeout = 0;
                    i2c_master_addressing(I2C0, Addr<<1, I2C_MASTER_TRANSMIT);
                    state = I2C_TRANSMIT_DATA;
                }else{
                    timeout = 0;
                    state = I2C_START;
                    //printf("i2c master sends 's internal address timeout in  writeByte!\r\n");
                }
                break;
            case I2C_TRANSMIT_DATA:
                while((!i2c_flag_get(I2C0, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)){
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT){
                    timeout = 0;
                    i2c_data_transmit(I2C0, *data);
                    state = I2C_STOP;
                }else{
                    timeout = 0;
                    state = I2C_START;
                    //printf("i2c master sends data timeout in  writeByte! \r\n");
                }
                //printf("i2c_step2 NCK:%d! \r\n",i2c_flag_get(I2C0, I2C_FLAG_NACK));
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
                    printf("i2c master sends stop signal timeout in  writeByte!\r\n");
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
    return state;
}

uint8_t sensirion_i2c_ReadByte_timeout(uint8_t Addr,uint8_t *data,uint16_t count){
    // IMPLEMENT
    #ifdef GD32E230
    uint8_t     state = I2C_START;
    uint16_t    timeout = 0;
    uint8_t     i2c_timeout_flag = 0;
    i2c_ackpos_config(I2C1, I2C_ACKPOS_NEXT);
    while(!(i2c_timeout_flag)){
        switch (state)
        {
        case I2C_START:
            while(i2c_flag_get(I2CX, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            if(timeout < I2C_TIME_OUT){
                i2c_start_on_bus(I2C1);//i2c_start
                timeout = 0;
                state = I2C_SEND_ADDRESS;
            }else{
                timeout = 0;
                state = I2C_START;
                //PR_DEBUG("i2c bus is busy in byte read \r\n");
            }
            break;
        case I2C_SEND_ADDRESS:
            /* i2c master sends START signal successfully */
            while((!i2c_flag_get(I2CX, I2C_FLAG_SBSEND)) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            if(timeout < I2C_TIME_OUT){
                i2c_master_addressing(I2CX, (Addr << 1), I2C_RECEIVER);
                timeout = 0;
                state = I2C_CLEAR_ADDRESS_FLAG;
            }else{
                timeout = 0;
                state = I2C_START;
                //add //提前退出循环
                i2c_timeout_flag = I2C_OK;
                //PR_DEBUG("i2c master sends start signal timeout in BYTE READ!\n");
            }
            break;
        case I2C_CLEAR_ADDRESS_FLAG:
            /* address flag set means i2c slave sends ACK */
            while((!i2c_flag_get(I2CX, I2C_FLAG_ADDSEND)) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            if(timeout < I2C_TIME_OUT){
                i2c_flag_clear(I2CX, I2C_FLAG_ADDSEND);
                timeout = 0;
                state = I2C_TRANSMIT_DATA;
            }else{
                timeout = 0;
                state = I2C_START;
                //add //提前退出循环
                i2c_timeout_flag = I2C_OK;
                //PR_DEBUG("i2c master clears address flag timeout in BYTE WRITE!\n");
            }
            break;
        case I2C_TRANSMIT_DATA:
             /* wait until the transmit data buffer is empty */
            while((!i2c_flag_get(I2CX, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            while(count){
                if(count == 1){
                    i2c_ackpos_config(I2C1, I2C_ACKPOS_CURRENT);
                    i2c_ack_config(I2C1, I2C_ACK_DISABLE);
                }
                if(i2c_flag_get(I2C1, I2C_FLAG_RBNE)){
                    *data = i2c_data_receive(I2C1);
                    i2c_ack_config(I2C1, I2C_ACK_ENABLE);
                    data++;
                    count--;
                }else{
                    state = I2C_START;
                    timeout = 0;
                }
            }
            state = I2C_STOP;
            timeout = 0;
            break;
        case I2C_STOP:
            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(I2CX);
            /* i2c master sends STOP signal successfully */
            while((I2C_CTL1(I2CX) & 0x0200) && (timeout < I2C_TIME_OUT)){
                timeout++;
            }
            if(timeout < I2C_TIME_OUT){
                i2c_ackpos_config(I2C1, I2C_ACKPOS_CURRENT);
                /* enable acknowledge */
                i2c_ack_config(I2C1, I2C_ACK_ENABLE);
                
                timeout = 0;
                state = I2C_END;
                i2c_timeout_flag = I2C_OK;
            }else{
                timeout = 0;
                state = I2C_START;
                //PR_DEBUG("i2c master sends stop signal timeout in BYTE READ\n");
            }
            break;
        default:
            state = I2C_START;
            i2c_timeout_flag = I2C_OK;
            timeout = 0;
            //PR_DEBUG("i2c master sends start signal in BYTE READ!\n");
            break;
        }
    }
    #endif

    #ifdef GD32L235
    i2c_process_enum    state = I2C_START;
    uint32_t            timeout = 0;
    uint8_t             end_flag = 0;
    //i2c_nack_disable(I2CX);
    i2c_transfer_byte_number_config(I2CX, count);
#if 1
    i2c_reload_disable(I2CX);
    /* enable I2C automatic end mode in master mode */
    i2c_automatic_end_enable(I2CX);
#endif

    while(!(end_flag))
    {
        //printf("i2c bus is busy in read_state:%d,LINE: %d!\r\n",state,__LINE__);
        switch (state)
        {
        case I2C_START:
            /* configure number of bytes to be transferred */
            while(i2c_flag_get(I2CX, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT)) 
            {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_start_on_bus(I2CX);
                timeout = 0;
                state = I2C_SEND_ADDRESS;
            } else {
                /* timeout, bus reset */
                //i2c_bus_reset();
                timeout = 0;
                state = I2C_START;
                end_flag = I2C_OK;
                printf("i2c bus is busy in read_%d!\r\n",__LINE__);
            }
            break;
        case I2C_SEND_ADDRESS:
            while((!i2c_flag_get(I2CX, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_master_addressing(I2CX, (Addr<<1), I2C_MASTER_RECEIVE);
                state = I2C_TRANSMIT_DATA;
            }else{
                timeout = 0;
                state = I2C_START;
                end_flag = I2C_OK;
                printf("i2c bus is I2C_SEND_ADDRESS timeout:%d!\r\n",__LINE__);
            }
            break;
        case I2C_TRANSMIT_DATA:
            while(count)
            {
                if(i2c_flag_get(I2CX, I2C_FLAG_RBNE)){
                    *data = i2c_data_receive(I2CX);
                    data++;
                    count--;
                }
            }
            state = I2C_STOP;
            timeout = 0;
            //printf("i2c transmit complete %d \r\n",i2c_flag_get(I2CX, I2C_FLAG_RBNE));
            break;
        case I2C_STOP:
            /* wait until the stop condition is finished */
            while((!i2c_flag_get(I2CX, I2C_FLAG_STPDET)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                /* clear STPDET flag */
                i2c_flag_clear(I2CX, I2C_FLAG_STPDET);
                timeout = 0;
                state = I2C_END;
                end_flag = I2C_OK;
            } else {
                timeout = 0;
                state = I2C_START;
                //printf("i2c master sends stop signal timeout in read!\n");
            }
            break;
        default:
            /* default status */
            state = I2C_START;
            end_flag = 1;
            timeout = 0;
            printf("i2c master sends start signal in read!\n");
            break;
        }
    }
    #endif
    return state;
}

void sensirion_i2c_gpio_config(){
    #ifdef GD32E230
    /* connect PA0 to I2C1_SCL */
    gpio_af_set(GPIOA, GPIO_AF_4, GPIO_PIN_0);
    /* connect PA1 to I2C1_SDA */
    gpio_af_set(GPIOA, GPIO_AF_4, GPIO_PIN_1);
    /* configure GPIO pins of I2C1 */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_0);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_1);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
    #endif
    #ifdef GD32L235
    /* connect PB8 to I2C0_SCL */
    gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_8);
    /* connect PB9 to I2C0_SDA */
    gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_9);
    /* configure GPIO pins of I2C0 */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_8);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    #endif
}

void sensirion_i2c_rcu_config(){
    /* enable GPIOB clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    /* enable I2C0 clock */
    rcu_periph_clock_enable(RCU_I2C0);
}

void sensirion_i2c_attribute_config(){
    #ifdef GD32E230
    /* I2C clock configure */
    i2c_clock_config(I2C1, 100000, I2C_DTCY_2);
    /* I2C address configure */
    i2c_mode_addr_config(I2C1, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, I2C1_OWN_ADDRESS7);
    /* enable I2C1 */
    i2c_enable(I2C1);
    /* enable acknowledge */
    i2c_ack_config(I2C1, I2C_ACK_ENABLE);
    #endif

    #ifdef GD32L235
    /* configure I2C timing */
    i2c_timing_config(I2C0, 0, 0x3, 0);
    i2c_master_clock_config(I2C0, 0x13, 0x36);
    i2c_address_config(I2C0, I2C1_OWN_ADDRESS7, I2C_ADDFORMAT_7BITS);
    /* configure slave address */
    //i2c_master_addressing(I2C0, 0x82, I2C_MASTER_TRANSMIT);
    /* enable I2C0 */
    i2c_enable(I2C0);
    #endif
}