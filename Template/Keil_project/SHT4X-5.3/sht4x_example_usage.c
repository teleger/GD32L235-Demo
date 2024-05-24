/*
 * Copyright (c) 2020, Sensirion AG
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

#include "sht4x.h"
#include <stdio.h>  // printf
#include <math.h>
#include "sht4x_usage_demo.h"

#ifdef GD32E230
#include "gd32e23x.h"
#endif

#ifdef GD32L235
#include "gd32l23x.h"
#endif
/**
 * TO USE CONSOLE OUTPUT (PRINTF) AND WAIT (SLEEP) PLEASE ADAPT THEM TO YOUR
 * PLATFORM
 */
#if 0
int mainSHT_53(void) {
    /* Initialize the i2c bus for the current platform */
    sensirion_i2c_init();

    /* Busy loop for initialization, because the main loop does not work without
     * a sensor.
     */
    while (sht4x_probe() != STATUS_OK) {
        PR_DEBUG("SHT sensor probing failed \r\n");
        sensirion_sleep_usec(1000000); /* sleep 1s */
    }
    PR_DEBUG("SHT sensor probing successful \r\n");

    while (1) {
        float temper = 0.00f;
        float humi = 0.00f;
        int32_t temperature, humidity;
        /* Measure temperature and relative humidity and store into variables
         * temperature, humidity (each output multiplied by 1000).
         */
        int8_t ret = sht4x_measure_blocking_read(&temperature, &humidity);
        if (ret == STATUS_OK) {
            PR_DEBUG("measured temperature: %0.2f degreeCelsius, "
                   "measured humidity: %0.2f percentRH \r\n",
                   temperature / 1000.0f, humidity / 1000.0f);
        } else {
            PR_DEBUG("error reading measurement\r\n");
        }

        sensirion_sleep_usec(1000000); /* sleep 1s */
    }
    return 0;
}
#endif

int SHT4x_init(void) {
    /* Initialize the i2c bus for the current platform and probe? if ok,then continue work?*/
    //sensirion_i2c_init();
    return sht4x_probe();
}

void  Init_SHT4X(void){
    int ret = SHT4x_init();
    if(ret != SHT4X_OK){
        set_sht4x_status(SHT4X_UNKOWN);
        printf("Init_SHT4X, Failed:%d\r\n",ret);
        return;
    }
    set_sht4x_status(STATUS_OK);
    printf("Init_SHT4X, OK\r\n");
}

//sleep(1);/* sleep 1s */
int8_t sht4x_measure_detail(int32_t *temperature, int32_t *humidity){
    int8_t ret = 0; 
    ret = sht4x_measure_blocking_read(temperature, humidity);
    return ret;
}