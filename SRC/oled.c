/********************* (C) COPYRIGHT 2016 e-Design Co.,Ltd. ********************
 File Name :      olde.c
 Version :        1.7a
 Description:
 Author :         Ning
 Data:            2017/11/22
 History:
 2017/06/20       新增不同硬件版本下的定义配置;
 2017/10/21       优化汇总iic配置;
 *******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "oled.h"
#include "hardware.h"
#include "iic.h"

uint8_t gOled_param[23] =
{
	0xAE,			// Set Display OFF
	0xD5, 0x52,		// Set Display Clock Divide Ratio/ Oscillator Frequency (D5h)
	0xA8, 0x0f,		// Set Multiplex Ratio (A8h)
	0xC8,			// Set COM output scan direction
	0xD3, 0x00,		// Set Display Offset (D3h)
	0x40,			// D/C#=1 write data
	0xA1, 			// Set Segment Re-map (A0h/A1h)
	0x8D, 0x14,		// Enable charge pump regulator
	0xDA, 0x02,		// Set COM Pins Hardware Configuration (DAh)
	0x81, 0x33,		// Set Contrast Control for BANK0 (81h)
	0xD9, 0xF1,		// Set Pre-charge Period (D9h)
	0xDB, 0x30,		// Set VCOMH  Deselect Level (DBh)
	0xA4, 0xA6,		// Entire Display ON   (A4h/A5h)
	0xAF			// Set Display ON
};

/*******************************************************************************
 函数名: Set_ShowPos
 函数作用:要显示内容的位置
 输入参数:x:横坐标,y:纵坐标(0,8,16,24)
 返回参数:NULL
 *******************************************************************************/
void OLED_Set_Pos(uint8_t x, uint8_t y)
{
	x += 0;
	Write_IIC_Command(0xb0 + y);
	Write_IIC_Command(((x & 0xf0) >> 4) | 0x10);
	Write_IIC_Command(x & 0x0f); //Write_IIC_Command((x&0x0f)|0x01);
}
/*******************************************************************************
 函数名: Oled_DrawArea
 函数作用:显示一个区域
 输入参数: x0:起始横坐标
 y0:起始纵坐标(0,8,16,24)
 wide:显示内容宽度
 high:显示内容高度
 ptr:显示的内容库指针
 返回参数:下一库指针
 *******************************************************************************/
uint8_t* Oled_DrawArea(uint8_t x0, uint8_t y0, uint8_t wide, uint8_t high, uint8_t *ptr)
{
	uint8_t m, n, y;

	n = y0 + high;
	if (y0 % 8 == 0)
		m = y0 / 8;
	else
		m = y0 / 8 + 1;

	if (n % 8 == 0)
		y = n / 8;
	else
		y = n / 8 + 1;

	//y是有多少行.
	//m是多少个8行,每多一个m值加一.
	for (; m < y; m++)
	{
		OLED_Set_Pos(x0, m);
		Write_IIC_PageData(ptr, wide);
		ptr += wide;
		//for(x=0; x<wide; x++) {
		//    Write_IIC_Data(*ptr++);
		//}
	}
	return ptr;
}
/*******************************************************************************
 函数名: Init_Oled
 函数作用:初始化LED设置
 输入参数:NULL
 返回参数:NULL
 *******************************************************************************/
void Init_Oled(void)
{
	uint32_t i;

	OLED_RST();
	Delay_Ms(2);
	OLED_ACT();
	Delay_Ms(2);

	Delay_Ms(105);
	for (i = 0; i < 23; i++)
	{
		Write_IIC_Command(gOled_param[i]);
	}
}

