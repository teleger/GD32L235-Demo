#include "gdflash.h"
#include <string.h>
//STMFLASH_ReadOneWord
u32 STMFLASH_ReadOneWord(u32 faddr)
{
	return *(vu32*)faddr; 
}

#if STM32_FLASH_WREN	//���ʹ����д   

//add
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ4�ı���!!)
//pBuffer:����ָ��
//NumToWrite:����(32λ)��(����Ҫд���32λ���ݵĸ���.)
void STMFLASH_WriteWord_NoCheck(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite)   
{ 			 		 
	u32 i;
	for(i=0;i<NumToWrite;i++)
	{
		#ifdef GD32L233
		fmc_word_program(WriteAddr,pBuffer[i]);
	    #endif
		WriteAddr+=4; //��ַ����4.
		fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR);
	}  
}

#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif
	 
u32 STMFLASH_BUF[STM_SECTOR_SIZE/4];//�����2K�ֽ�

#endif

//add---STMFLASH_ReadWord
void STMFLASH_ReadWord(u32 ReadAddr,u32 *pBuffer,u32 NumToRead){
	u32 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadOneWord(ReadAddr);//��ȡ4���ֽ�.
		ReadAddr+=4;//ƫ��4���ֽ�.	
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//WriteAddr:��ʼ��ַ
//WriteData:Ҫд�������
#if 0
void Test_Write(u32 WriteAddr,u32 WriteData)   	
{
	STMFLASH_WriteWord(WriteAddr,&WriteData,1);//д��һ���� 
}
void Test_read(){
	u32 buffer[2]={0};
	STMFLASH_ReadWord(0x0800F000,buffer,1);
	printf("test,read:0x%x\r\n",buffer[0]);
}
#endif

void WriteFlash(u32 address,u8 *pBuffer,u32 Numlengh)
{
	u32 i,temp;
  	fmc_unlock();
	for(i = 0; i < Numlengh;i+= 4)
	{
		temp =  (uint32_t)pBuffer[i+3]<<24;
		temp |=  (uint32_t)pBuffer[i+2]<<16;
		temp |=  (uint32_t)pBuffer[i+1]<<8;
		temp |=  (uint32_t)pBuffer[i];
		#ifdef GD32L233
		fmc_word_program(address+i,temp);
		#endif
		fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR);
	}
  	fmc_lock();
}

void ReadFlash(u32 address,uint8_t *Nbuffer,u32  length)
{
	u32 temp = 0;
	u32 count = 0;
	while(count < length)
	{
		temp = *((volatile uint32_t *)address);//ȡ��ַ�е�ֵ���Ĵ������32λ
		*Nbuffer++ = (temp & 0xff);//ȡ��8λ�ŵ�����[0]
		count++;
    if(count >= length)
      break;
	  *Nbuffer++ = (temp >> 8) & 0xff;//����8λ ȡ��8λ�ŵ�����[1]
		count++;
		if(count >= length)
      break;
		*Nbuffer++ = (temp >> 16) & 0xff;//����8λ ȡ��8λ�ŵ�����[2]
		count++;
		if(count >= length)
      break;
		*Nbuffer++ = (temp >> 24) & 0xff;//����8λ ȡ��8λ�ŵ�����[3] 32λ����ȡ��
		count++;
		if(count >= length)
      break;
		address += 4;//��ַ + 4 ������һ��32λ
	}
}

u8 EarseFlash_1K(uint32_t address){
	fmc_unlock();
	if(address % 1024 == 0)
	{
		fmc_page_erase(address);
	}
	else
	{
		return 0;
	}
	fmc_lock();
	return 1;
}

u8 cBuffer[1024];

void update_firmware(u32 SourceAddress,u32 TargetAddress)
{
  	u16 i;
	volatile u32 nK;
	nK	= (TargetAddress - SourceAddress)/1024;
	for(i = 0;i < nK;i++)
	{
		memset(cBuffer,0xff,sizeof(cBuffer));
		ReadFlash(TargetAddress+i*0x0400,cBuffer,1024);
		EarseFlash_1K(SourceAddress+i*0x0400);
		WriteFlash(SourceAddress+i*0x0400,cBuffer,1024);
	}
}









