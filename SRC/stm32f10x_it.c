/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "hardware.h"

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) {
    }
}

void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1) {
    }
}


void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1) {
    }
}

void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1) {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
}

void TIM2_IRQHandler(void)
{
	/*10ms一个周期*/
	static uint8_t sk = 0; //scan key
	uint8_t i;
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	   // 中断标志复位

	if (sk++ == 2)
	{	   //20ms一次
		Key_Read();
		sk = 0;
	}
	if (HWVER_1P4 == version_number)
	{
		gKey_Press = KEY_READ ? 1 : 0;
	}
	else
	{
		gKey_Press = KEY_READ_1 ? 1 : 0;
	}

	for (i = 0; i < 6; i++)
	{
		if (gTimer[i] > 0)
		{
			gTimer[i]--;
		}
	}

	Timer_Counter++;
}

void ADC1_2_IRQHandler(void)
{
	if (ADC_GetITStatus(ADC1, ADC_IT_AWD))
	{
		AWD_entry++;

		if (gTimer[0] == 0) // && Is_Turbo_On())
		{
			TURBO_OFF();
			//AWD_entry = 0;
		}

		GPIO_SetBits(GPIOB, GPIO_Pin_2);
		// Clear ADC1 AWD pending interrupt bit
		ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
	}
}

void DMA1_Channel1_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC1) != RESET)
	{
		ADC_filter();
		DMA_ClearITPendingBit (DMA1_IT_TC1);
	}
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    USB_Istr();
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/*********************************END OF FILE*********************************/
