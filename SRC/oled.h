/********************* (C) COPYRIGHT 2016 e-Design Co.,Ltd. ********************
File Name :      olde.h
Version :        1.7a
Description:
Author :         Ning
Data:            2017/11/22
History:
2017/10/21       ¥˙¬Î”≈ªØ;
*******************************************************************************/
#ifndef _OLED_SSD1306_H
#define _OLED_SSD1306_H
#include "stm32f10x.h"

#define OLED_RST()              GPIO_ResetBits(GPIOB, GPIO_Pin_0)
#define OLED_ACT()              GPIO_SetBits  (GPIOB, GPIO_Pin_0)

void OLED_Set_Pos(uint8_t x, uint8_t y);
uint8_t* Oled_DrawArea(uint8_t x0, uint8_t y0, uint8_t wide, uint8_t high, uint8_t* ptr);
void Init_Oled(void);
void Clear_Screen(void);

#endif
/******************************** END OF FILE *********************************/
