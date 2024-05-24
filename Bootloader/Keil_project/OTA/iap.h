#ifndef __IAP_H__
#define __IAP_H__

#include "ota_data_type.h"
typedef  void (*iapfun)(void);				

#define BOOTLOADER          (0x08000000)		 //address:                   				       
#define APP_CODE_OFFSET     (0x08002C00)         //APP 起始地址
#define APP_CODE_SIZE       (0x6800)             //26KB
#define NEW_APP_ADDR       	(0x08009400)	     //OTA new bin address
#define OTA_BUF_LEN         (0x6800)		     //max size 26kb
#define FLASH_SECTOR_SIZE   (0x400)			     //1KB

//bootloader的Last 1kb 存放 VERSION_INFO AND REMOTE_INFO
#define APP_FIRMWARE_TEST_ADDRESS (0x08002800) 	 //测试标志
#define APP_CONVTEMP_FLAG_ADDRESS (0x08002804)
#define APP_VERSION_ADDRESS (0x08002808)
#define APP_VERSION_SIZE_MAX (0x0A)
#define APP_REMOTE_INFO_ADDRESS (0x08002810)
#define APP_REMOTE_INFO_SIZE_MAX (0x40)// 实际只有12个,最长存16个字 (begin address+16*4) 
//END

#define OTA_FLAG_ADDR       (0x0800FC00)		 //address: 11-12kb; 64kb flash中,最后1kb 开始地址
#define APP_LEN_ADDR		(0x0800FC04)
	  
typedef enum{
	Clear_IAP_Flag,
	Set_IAP_flag,
}IAP_FLAG_E;

void iap_load_app(u32 appxaddr);
#if 0
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	
#endif

void MSR_MSP(u32 addr);
void ota_process();
void ConfigureAppUpdateflag(IAP_FLAG_E e);
#endif







































