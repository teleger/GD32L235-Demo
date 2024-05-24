#ifdef GD32E230
	 #include "gd32e23x.h"
#else
	#include "gd32l23x.h"
#endif

#include "gdflash.h"
#include "iap.h"
								  
iapfun  jump2app; 
u32 		iapbuf[256];   

#if 0
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u32 t;
	u16 i=0;
	u32 temp;
	u32 fwaddr=appxaddr;
	u8 *dfu=appbuf;
	for(t=0;t<appsize;t+=4)
	{						    	
		temp =  (uint32_t)dfu[3]<<24;//最高位 0xff000000
		temp |=  (uint32_t)dfu[2]<<16;//0x00ff0000
		temp |=  (uint32_t)dfu[1]<<8;//0x0000ff00
		temp |=  (uint32_t)dfu[0];//最低位0x000000ff

		dfu+=4;
		iapbuf[i++]=temp;	    
		if(i==256)
		{
			i=0;
			STMFLASH_WriteWord(fwaddr,iapbuf,256);	
			fwaddr+=1024;
		}
	}
	if(i)STMFLASH_WriteWord(fwaddr,iapbuf,i);
}
#endif

void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	
	{ 
		#if BOOTLOADER_UART_ENABLE
			printf("iap_load_app:0x%x\r\n",appxaddr);
			usart_disable(USART1);
		#endif
		rcu_deinit();
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		
		__set_MSP(*(vu32*) appxaddr); 					
		jump2app();
	}else{
		#if BOOTLOADER_UART_ENABLE
		printf("iap_load:0x%x,fail_iap_load_app_value:0x%x\r\n",appxaddr,(*(vu32*)appxaddr));
		#endif
	}
}

unsigned char nbuff_AAAA[] = {0xAA,0xAA,0xAA,0xAA};
unsigned char nbuff_5555[] = {0x55,0x55,0x55,0x55};

void ConfigureAppUpdateflag(IAP_FLAG_E e){
	EarseFlash_1K(OTA_FLAG_ADDR);
	switch (e)
	{
	case Set_IAP_flag:
		WriteFlash(OTA_FLAG_ADDR,nbuff_AAAA,1);
		break;
	case Clear_IAP_Flag:
		WriteFlash(OTA_FLAG_ADDR,nbuff_5555,1);
		break;
	default:
		break;
	}
}

void ota_process(){
	#if BOOTLOADER_UART_ENABLE
		printf("<<<bootloader>>>\r\n");
	#endif
	u32 	applenth = 0x00;//待升级的app代码长度
	u32  	ota_flag =*(uint32_t *)OTA_FLAG_ADDR;// OTA标志
	#if BOOTLOADER_UART_ENABLE
		printf("ota:0x%x\r\n",ota_flag);
	#endif
	if(ota_flag == 0xAAAAAAAA){//更新
		applenth =*(uint32_t *)APP_LEN_ADDR;				//待升级的app代码长度
		#if BOOTLOADER_UART_ENABLE
		printf("begin update firmware...\r\n");
		printf("new,APP_Length:0x%x\r\n",applenth);
		#endif
		//拷贝固件
		update_firmware(APP_CODE_OFFSET,NEW_APP_ADDR);//更新BIN文件代码到APP block
		//printf("firm update completed!\r\n");
		//清除升级标志
		ConfigureAppUpdateflag(Clear_IAP_Flag);
		#if BOOTLOADER_UART_ENABLE
		printf("new version:%d.%d.%d\r\n",*(u32 *)APP_VERSION_ADDRESS,*(u32 *)APP_VERSION_ADDRESS+4,*(u32 *)APP_VERSION_ADDRESS+8);
		printf(">>>runNewAPP>>>>\r\n");
		#endif	
		iap_load_app(APP_CODE_OFFSET);//执行APP block代码
	}else{
		iap_load_app(APP_CODE_OFFSET);//执行APP block代码
	}
}





