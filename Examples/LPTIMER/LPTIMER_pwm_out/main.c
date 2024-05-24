/*!
    \file    main.c
    \brief   LPTIMER PWM out example

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
#include <stdio.h>
#include "gd32l23x_eval.h"
#include "main.h"

/* configure the GPIO ports */
void gpio_config(void);
/* configure the LPTIMER peripheral */
void lptimer_config(void);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/

int main(void)
{
    /* configure the GPIO ports */
    gpio_config();
    /* configure the LPTIMER peripheral */
    lptimer_config();

    while(1) {
    }
}

/*!
    \brief      configure the GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
*/
void gpio_config(void)
{
    /* LPTIMERN GPIO RCU */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* LPTIMERN GPIO */
    /*configure PA4(LPTIMERN_O) as alternate function*/
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    gpio_af_set(GPIOA, GPIO_AF_2, GPIO_PIN_4);
}

/*!
    \brief      configure the LPTIMER peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void lptimer_config(void)
{
    /* ----------------------------------------------------------------------------
    LPTIMERN Configuration:
    LPTIMERN count with internal clock IRC32K, the prescaler is 16, the period is 1000.

    LPTIMERN configuration: generate 1 PWM signal with the duty cycle:
    LPTIMERCLK = IRC32K / 16 = 2KHz

    LPTIMERN_O duty cycle = (500/ 1000)* 100  = 50%
    ---------------------------------------------------------------------------- */
    /* LPTIMERN configuration */
    lptimer_parameter_struct lptimer_structure;
    /* LPTIMERN clock */
#ifdef GD32L233
    rcu_periph_clock_enable(RCU_LPTIMER);
#else
    rcu_periph_clock_enable(RCU_LPTIMER0);
#endif
    rcu_osci_on(RCU_IRC32K);
    rcu_osci_stab_wait(RCU_IRC32K);
#ifdef GD32L233
    rcu_lptimer_clock_config(RCU_LPTIMERSRC_IRC32K);
#else
    rcu_lptimer_clock_config(IDX_LPTIMER0, RCU_LPTIMERSRC_IRC32K);
#endif
    /* deinit a LPTIMER */
    lptimer_deinit(LPTIMERN);
    /* initialize LPTIMERN init parameter struct */
    lptimer_struct_para_init(&lptimer_structure);
    /* LPTIMERN configuration */
    lptimer_structure.clocksource      = LPTIMER_INTERNALCLK;
    lptimer_structure.prescaler        = LPTIMER_PSC_16;
    lptimer_structure.extclockpolarity = LPTIMER_EXTERNALCLK_RISING;
    lptimer_structure.extclockfilter   = LPTIMER_EXTERNALCLK_FILTEROFF;
    lptimer_structure.triggermode      = LPTIMER_TRIGGER_SOFTWARE;
    lptimer_structure.extriggersource  = LPTIMER_EXTRIGGER_GPIO;
    lptimer_structure.extriggerfilter  = LPTIMER_TRIGGER_FILTEROFF;
    lptimer_structure.outputpolarity   = LPTIMER_OUTPUT_NOTINVERTED;
    lptimer_structure.outputmode       = LPTIMER_OUTPUT_PWMORSINGLE;
    lptimer_structure.countersource    = LPTIMER_COUNTER_INTERNAL;
    lptimer_init(LPTIMERN, &lptimer_structure);

    lptimer_register_shadow_disable(LPTIMERN);
    lptimer_timeout_disable(LPTIMERN);
    lptimer_countinue_start(LPTIMERN, 999U, 500U);

    /* wait EXTI triggle start LPTIMERN counter*/
    while(lptimer_counter_read(LPTIMERN) == 0) {
    }
}
