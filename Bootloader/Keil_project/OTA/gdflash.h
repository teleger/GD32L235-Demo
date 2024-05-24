#ifndef __STMFLASH_H__
#define __STMFLASH_H__

#ifdef GD32E230
    #include "gd32e23x.h"
#else
    #include "gd32l23x.h"
#endif
#include "ota_data_type.h"

#define STM32_FLASH_SIZE 64 	 		
#define STM32_FLASH_WREN 1              

#define STM32_FLASH_BASE 0x08000000 	
void    STMFLASH_WriteWord(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite);		
void    STMFLASH_ReadWord(u32 ReadAddr,u32 *pBuffer,u32 NumToRead);
void    Test_Write(u32 WriteAddr,u32 WriteData);
void    Test_read();
u8      EarseFlash_1K(uint32_t address);
//add
void    WriteFlash(u32 address,u8 *pBuffer,u32 Numlengh);
void    ReadFlash(u32 address,u8 *Nbuffer,u32  length);
void    update_firmware(u32 SourceAddress,u32 TargetAddress);						   
#endif

















