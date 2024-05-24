/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_DEBUG_H__
#define __MAIN_DEBUG_H__

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdio.h"
/* USER CODE BEGIN Includes */
typedef struct
{
	uint8_t Hour;
	uint8_t Min;
}Time_typedef;
typedef struct
{
	Time_typedef Time;
	uint8_t Temp;
}Data;
typedef struct
{
	Data ONE;
	Data TWO;
	Data THR;
	Data FOU;
	Data FIV;
	Data SIX;
}Interval;
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define KEY1_Pin GPIO_PIN_11
#define KEY1_GPIO_Port GPIOA
#define KEY2_Pin GPIO_PIN_12
#define KEY2_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_5
#define LED2_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_6
#define LED1_GPIO_Port GPIOB
#define LED_G_Pin GPIO_PIN_7
#define LED_G_GPIO_Port GPIOB
#define LED_R_Pin GPIO_PIN_8
#define LED_R_GPIO_Port GPIOB
#define LED_B_Pin GPIO_PIN_9
#define LED_B_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

#ifndef GD32L235
#define GD32L235
#endif



//打印PR_DEBUG调试信息 开关
#define DEBUG_MODE              0

//=============================================================================
//打印信息
//=============================================================================
#if (0==DEBUG_MODE)
#define PR_DEBUG(_fmt_, ...)
#define PR_INFO(_fmt_, ...)
#define PR_NOTICE(_fmt_, ...)   
#define PR_ERR(_fmt_, ...)      
#define PR_DATA(_fmt_, ...) 
#elif (1==DEBUG_MODE)
#define PR_DEBUG(_fmt_, ...)
#define PR_NOTICE(_fmt_, ...)   printf("[NOTICE][%s:%d] "_fmt_"", strrchr(__FILE__,'\\')+1, __LINE__, ##__VA_ARGS__);
#define PR_ERR(_fmt_, ...)      printf("[ERR][%s:%d] "_fmt_"", strrchr(__FILE__,'\\')+1, __LINE__, ##__VA_ARGS__); 
#define PR_DATA(_fmt_, ...) 
#elif (2==DEBUG_MODE)
#define PR_DEBUG(_fmt_, ...)    printf("[DEBUG][%s:%d] "_fmt_"", strrchr(__FILE__,'\\')+1, __LINE__, ##__VA_ARGS__);
#define PR_NOTICE(_fmt_, ...)   printf("[NOTICE][%s:%d] "_fmt_"", strrchr(__FILE__,'\\')+1, __LINE__, ##__VA_ARGS__);
#define PR_ERR(_fmt_, ...)      printf("[ERR][%s:%d] "_fmt_"", strrchr(__FILE__,'\\')+1, __LINE__, ##__VA_ARGS__);
#define PR_DATA(_fmt_, ...)     printf(""_fmt_"",##__VA_ARGS__); 
#elif (3== DEBUG_MODE)
#define PR_DEBUG(_fmt_, ...)
#define PR_NOTICE(_fmt_, ...)   printf("[NOTICE][%s:%d] "_fmt_"", strrchr(__FILE__,'\\')+1, __LINE__, ##__VA_ARGS__);
#define PR_ERR(_fmt_, ...)      
#define PR_DATA(_fmt_, ...)  
#elif (4 == DEBUG_MODE)
#define PR_DEBUG(_fmt_, ...) printf("[DEBUG][%s:%d] "_fmt_"", strrchr(__FILE__,'\\')+1, __LINE__, ##__VA_ARGS__);
#define PR_NOTICE(_fmt_, ...)   
#define PR_ERR(_fmt_, ...)      
#define PR_DATA(_fmt_, ...)  
#elif (5 == DEBUG_MODE)
#define PR_DEBUG(_fmt_, ...) 
#define PR_INFO(_fmt_, ...)  printf("[INFO][%s:%d] "_fmt_"", strrchr(__FILE__,'\\')+1, __LINE__, ##__VA_ARGS__);
#define PR_NOTICE(_fmt_, ...)   
#define PR_ERR(_fmt_, ...)      
#define PR_DATA(_fmt_, ...)  
#elif (6 == DEBUG_MODE)
#define PR_DEBUG(_fmt_, ...)
#define PR_NOTICE(_fmt_, ...)   
#define PR_ERR(_fmt_, ...)      
#define PR_DATA(_fmt_, ...)  printf(""_fmt_"",##__VA_ARGS__); 
#else 
#define PR_DEBUG(_fmt_, ...)
#define PR_NOTICE(_fmt_, ...)   
#define PR_ERR(_fmt_, ...)      printf("[ERR][%s:%d] "_fmt_"", strrchr(__FILE__,'\\')+1, __LINE__, ##__VA_ARGS__);
#define PR_DATA(_fmt_, ...) 
#endif


//add
typedef struct  board_resources_status
{
  uint8_t cbu;//cbu模块
  uint8_t remote;//红外模块
  uint8_t led;//IIC Led
  uint8_t sht;//温湿度传感
  uint8_t button;//按钮
}board_resources_status_S;

//end



/* USER CODE BEGIN Private defines */
void UART1_SendByte(unsigned char data);
void UART3_SendByte(unsigned char data);
/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
