#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#ifdef GD32E230
	#include "gd32e23x.h"
#else
	#include "gd32l23x.h"
#endif

#include <stdio.h>
#include "systick.h"
#include "iap.h"

#define BOOTLOADER          (0x08000000)
#define BOOTLOADER_BIN_SIZE (0x2C00)  //11kb

#endif
