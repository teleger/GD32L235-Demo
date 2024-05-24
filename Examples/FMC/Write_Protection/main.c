/*!
    \file    main.c
    \brief   main flash program, write_protection

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

#define FLASH_PAGE_PROGRAM
//#define WRITE_PROTECTION_ENABLE
#define WRITE_PROTECTION_DISABLE

#ifdef GD32L235
    #define FLASH_PAGE_SIZE        ((uint16_t)0x0400U)
    #define FMC_PAGES_PROTECTED    (OB_WP_14 | OB_WP_15)
    #define BANK_WRITE_START_ADDR  ((uint32_t)0x08005000U)
    #define BANK_WRITE_END_ADDR    ((uint32_t)0x08005800U)
    uint64_t data = 0x0123456789abcdefU;
#else
    #define FLASH_PAGE_SIZE        ((uint16_t)0x1000U)
    #define FMC_PAGES_PROTECTED    (OB_WP_5 | OB_WP_6)
    #define BANK_WRITE_START_ADDR  ((uint32_t)0x08005000U)
    #define BANK_WRITE_END_ADDR    ((uint32_t)0x08007000U)
    uint32_t data = 0x01234567U;
#endif

typedef enum {FAILED = 0, PASSED = !FAILED} test_state;
uint32_t erase_counter = 0x0U, address = 0x0U;
uint32_t wp_value = 0xFFFFFFFFU, protected_pages = 0x0U;
uint32_t NbrOfPage;
__IO fmc_state_enum fmc_state = FMC_READY;
__IO test_state program_state = PASSED;

/* clear pengding flags */
void fmc_flags_clear(void);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* initialize leds on the board */
    gd_eval_led_init(LED1);
    gd_eval_led_init(LED2);

    /* unlock the flash program/erase controller */
    fmc_unlock();
    ob_unlock();

    fmc_flags_clear();

    /* get pages write protection status */
    wp_value = ob_write_protection_get();

#ifdef WRITE_PROTECTION_DISABLE
    /* get pages already write protected */
    protected_pages = ~(wp_value | FMC_PAGES_PROTECTED);

    /* check if desired pages are already write protected */
    if((wp_value | (~FMC_PAGES_PROTECTED)) != 0xFFFFFFFF) {
        /* erase all the option Bytes */
        fmc_state = ob_erase();

        /* check if there is write protected pages */
        if(protected_pages != 0x0) {
            /* restore write protected pages */
            fmc_state = ob_write_protection_enable(protected_pages);
        }
        /* generate System Reset to load the new option byte values */
        NVIC_SystemReset();
    }

#elif defined WRITE_PROTECTION_ENABLE
    /* get current write protected pages and the new pages to be protected */
    protected_pages = (~wp_value) | FMC_PAGES_PROTECTED;

    /* check if desired pages are not yet write protected */
    if(((~wp_value) & FMC_PAGES_PROTECTED) != FMC_PAGES_PROTECTED) {

        /* erase all the option Bytes because if a program operation is
        performed on a protected page, the Flash memory returns a
        protection error */
        fmc_state = ob_erase();

        /* enable the pages write protection */
        fmc_state = ob_write_protection_enable(protected_pages);

        /* generate System Reset to load the new option byte values */
        NVIC_SystemReset();
    }
#endif /* WRITE_PROTECTION_DISABLE */

#ifdef FLASH_PAGE_PROGRAM
    /* get the number of pages to be erased */
    NbrOfPage = (BANK_WRITE_END_ADDR - BANK_WRITE_START_ADDR) / FLASH_PAGE_SIZE;

    /* the selected pages are not write protected */
    if((wp_value & FMC_PAGES_PROTECTED) != 0x00) {
        /* clear all pending flags */
        fmc_flags_clear();

        /* erase the FLASH pages */
        for(erase_counter = 0; (erase_counter < NbrOfPage) && (fmc_state == FMC_READY); erase_counter++) {
            fmc_state = fmc_page_erase(BANK_WRITE_START_ADDR + (FLASH_PAGE_SIZE * erase_counter));
        }

        /* FLASH program of data at addresses defined by BANK_WRITE_START_ADDR and BANK_WRITE_END_ADDR */
        address = BANK_WRITE_START_ADDR;

        while((address < BANK_WRITE_END_ADDR) && (fmc_state == FMC_READY)) {
#ifdef GD32L235
            fmc_state = fmc_doubleword_program(address, data);
            address = address + 8;
#endif /* GD32L235 */
#ifdef GD32L233
            fmc_state = fmc_word_program(address, data);
            address = address + 4;
#endif /* GD32L233 */
        }

        /* check the correctness of written data */
        address = BANK_WRITE_START_ADDR;

        while((address < BANK_WRITE_END_ADDR) && (program_state != FAILED)) {
#ifdef GD32L235
            if((*(__IO uint64_t *) address) != data) {
                program_state = FAILED;
            }
            address += 8;
#endif /* GD32L235 */
#ifdef GD32L233
            if((*(__IO uint32_t *) address) != data) {
                program_state = FAILED;
            }
            address += 4;
#endif /* GD32L233 */
        }
        gd_eval_led_on(LED1);
    } else {
        /* error to program the flash : the desired pages are write protected */
        program_state = FAILED;
        gd_eval_led_on(LED2);

    }
#endif /* FLASH_PAGE_PROGRAM */
    while(1) {
    }
}


/*!
    \brief      clear pengding flags
    \param[in]  none
    \param[out] none
    \retval     none
*/
void fmc_flags_clear(void)
{
    fmc_flag_clear(FMC_FLAG_END);
    fmc_flag_clear(FMC_FLAG_WPERR);
    fmc_flag_clear(FMC_FLAG_PGAERR);
    fmc_flag_clear(FMC_FLAG_PGERR);
}
