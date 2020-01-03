/********************* (C) COPYRIGHT 2016 e-Design Co.,Ltd. ********************
File Name :      UI.h
Version :        1.7a
Description:
Author :         Ning
Data:            2017/11/22
History:
*******************************************************************************/
#ifndef _UI_H_
#define _UI_H_

#include "stm32f10x.h"

void Clear_Screen(void);
void Display_Str(uint8_t x, uint8_t y, char* str);
void Show_BatV(uint8_t step, uint8_t Charging);
void Show_Screw(uint8_t mode);
void Low_Power(void);
void Show_SetLv(uint8_t gear);
void Clear_Screw(void);
void Show_ScrewMove(uint8_t status);

#endif

