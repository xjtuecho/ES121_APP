/********************* (C) COPYRIGHT 2016 e-Design Co.,Ltd. ********************
File Name :      Hardware.h
Version :        1.7a
Description:
Author :         Ning
Data:            2017/11/22
History:
2017/06/20       新增硬件版本判断功能;
*******************************************************************************/
#ifndef _HARDWARE_H
#define _HARDWARE_H

#include "stm32f10x.h"
#include "stm32f10x_dma.h"

#define TURBO_ON()  GPIO_SetBits(GPIOA,GPIO_Pin_10);
//#define TURBO_OFF() GPIO_ResetBits(GPIOA,GPIO_Pin_10);

#define POWER_ON()  GPIO_SetBits(GPIOB,GPIO_Pin_4)
#define POWER_OFF() GPIO_ResetBits(GPIOB,GPIO_Pin_4)

#define POWER_ON_1()  GPIO_SetBits(GPIOD,GPIO_Pin_0)
#define POWER_OFF_1() GPIO_ResetBits(GPIOD,GPIO_Pin_0)

#define KEY_READ     (GPIOB->IDR & GPIO_Pin_7) //按下为1  1.4版本
#define KEY_READ_1   (GPIOA->IDR & GPIO_Pin_1) //按下为1  1.3版本

#define SHUTDOWN_TIME   1500    //关机时间单位10毫秒

#define NO_KEY    0x0000
#define KEY_V1    0x0080
#define KEY_CN    0X8000

#define USB_DN_OUT()    GPIOA->CRH = (GPIOA->CRH & 0xFFFF3FFF) | 0x00003000
#define USB_DP_OUT()    GPIOA->CRH = (GPIOA->CRH & 0xFFF3FFFF) | 0x00030000

#define USB_DN_EN()     GPIOA->CRH = (GPIOA->CRH & 0xFFFFBFFF) | 0x0000B000
#define USB_DP_EN()     GPIOA->CRH = (GPIOA->CRH & 0xFFFBFFFF) | 0x000B0000

#define USB_DP_PD()     GPIOA->CRH = (GPIOA->CRH & 0xFFF3FFFF) | 0x00030000

#define USB_DN_LOW()    GPIOA->BRR = GPIO_Pin_11
#define USB_DP_LOW()    GPIOA->BRR = GPIO_Pin_12

#define READ_CHG        (GPIOB->IDR & GPIO_Pin_6)

#define SECTOR_SIZE     512
#define SECTOR_CNT      4096

#define SERIAL_NO1         (*(uint32_t*)0x1FFFF7E8)
#define SERIAL_NO2         (*(uint32_t*)0x1FFFF7EC)
#define SERIAL_NO3         (*(uint32_t*)0x1FFFF7F0)

//--------------------------------- RCC 设置 ---------------------------------//

#define RCC_PLL_EN()        RCC->CR   |= 0x01000000;// PLL En

#define RCC_CFGR_CFG()      RCC->CFGR |= 0x0068840A;/*RCC peripheral clock config
                                           |||||||+--Bits3~0 = 1010 PLL used as sys clock
                                           ||||||+---Bits7~4 = 0000 AHB clock = SYSCLK
                                           |||||+----Bits10~8  = 100 PCLK1=HCLK divided by 2
                                           ||||++----Bits13~11 = 000 PCLK2=HCLK
                                           ||||+-----Bits15~14 = 10 ADC prescaler PCLK2 divided by 6
                                           |||+------Bit17~16 = 00 HSI/2 clock selected as PLL input clock
                                           ||++------Bits21~18 = 1010 PLL input clock x12
                                           ||+-------Bit22 = 1 USB prescaler is PLL clock
                                           ++--------Bits31~27 Reserved*/

extern volatile uint32_t gTimer[7];
extern volatile uint8_t gKey_Press;
extern uint8_t keypress_cnt;
extern uint16_t AWD_entry;

extern uint32_t gKey_state;
extern uint32_t gKey_gap;

#define HWVER_1P4		0
#define HWVER_1P3		1
extern uint8_t version_number;

#define SYSCLK_48M		1
#define SYSCLK_8M		0
extern uint8_t frequency_pos;

extern volatile uint32_t Timer_Counter;

void NVIC_Configuration(void);
void RCC_Config(void);

void USB_Port(uint8_t state);

uint32_t Start_Watchdog(uint32_t ms);
#define KickDog()	IWDG_ReloadCounter()

void Init_Gtime(void);
void Init_Timer2(void);
void GPIO_Config(void);
void GPIO_Config_1(void);
void Key_Read( void );
uint32_t Elapse_Value(uint32_t start,uint32_t cur);
void PWM_Init(uint16_t arr,uint16_t psc,uint8_t version);
void Version_Modify(void);
void TURBO_OFF(void);
void ADC_filter(void);

uint8_t Hardware_version(void);

void Delay_Init(void);
void Delay_Ms(uint16_t nms);

#define SI_COE         16		// 56
#define SI_THRESHOLD   60

#define VIN			0  //输入电压
#define VBTY		1  //电池电压
#define VCUR		2  //电流
#define ADC1_DR_Address ((uint32_t)0x4001244C)

void Adc_Init(void);
uint16_t Get_Adc(uint8_t channel);
void Set_CurLimit(uint16_t high_ma, uint16_t low_ma);

#endif
/******************************** END OF FILE *********************************/
