#ifndef __SPIO__H__
#define __SPIO__H__

#ifdef GD32L235 
#include "gd32l23x.h"
#endif

void spi0_rcu_config(void);
void spi0_gpio_config(void);
void spi0_config(void);
void spi0_init();

#endif