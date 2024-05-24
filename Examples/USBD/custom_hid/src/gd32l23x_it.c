/*!
    \file    gd32l23x_it.c
    \brief   interrupt service routines

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

#include "gd32l23x_it.h"
#include "usbd_lld_int.h"
#include "gd32l23x_eval.h"

__ALIGNED(2) uint8_t send_buffer1[4] = {0x00, 0x01, 0x00, 0x00};
__ALIGNED(2) uint8_t send_buffer2[4] = {0x00, 0x01, 0x00, 0x00};

extern uint8_t custom_hid_report_send(usb_dev *udev, uint8_t *report, uint16_t len);
extern usb_dev usbd_custom_hid;

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while(1) {
    }
}
/*!
    \brief      this function handles EXTI0_IRQ Handler.
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EXTI0_IRQHandler(void)
{
    if(RESET != exti_interrupt_flag_get(WAKEUP_KEY_EXTI_LINE)) {
        if(USBD_CONFIGURED == usbd_custom_hid.cur_status) {
            send_buffer1[0] = 0x15U;

            if(RESET != gd_eval_key_state_get(KEY_WAKEUP)) {
                if(send_buffer1[1]) {
                    send_buffer1[1] = 0x00U;
                } else {
                    send_buffer1[1] = 0x01U;
                }
            }

            custom_hid_report_send(&usbd_custom_hid, send_buffer1, 2U);
        }

        /* clear the EXTI line interrupt flag */
        exti_interrupt_flag_clear(WAKEUP_KEY_EXTI_LINE);
    }
}
/*!
    \brief      this function handles EXTI10_15_IRQ Handler.
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EXTI10_15_IRQHandler(void)
{
    if(RESET != exti_interrupt_flag_get(TAMPER_KEY_EXTI_LINE)) {
        if(USBD_CONFIGURED == usbd_custom_hid.cur_status) {
            send_buffer2[0] = 0x16U;

            if(RESET != gd_eval_key_state_get(KEY_TAMPER)) {
                if(send_buffer2[1]) {
                    send_buffer2[1] = 0x00U;
                } else {
                    send_buffer2[1] = 0x01U;
                }
            }

            custom_hid_report_send(&usbd_custom_hid, send_buffer2, 2U);
        }

        /* clear the EXTI line interrupt flag */
        exti_interrupt_flag_clear(TAMPER_KEY_EXTI_LINE);
    }
}
/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PendSV_Handler(void)
{
}

#ifdef GD32L235

/*!
    \brief      this function handles USBD low priority interrupt
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USBD_LP_CAN_RX0_IRQHandler(void)
{
    usbd_isr();
}

#else

/*!
    \brief      this function handles USBD low priority interrupt
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USBD_LP_IRQHandler(void)
{
    usbd_isr();
}

#endif /* GD32L235 */

#ifdef USBD_LOWPWR_MODE_ENABLE

/*!
    \brief      this function handles USBD wakeup interrupt request.
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USBD_WKUP_IRQHandler(void)
{
    exti_interrupt_flag_clear(EXTI_18);
}

#endif /* USBD_LOWPWR_MODE_ENABLE */
