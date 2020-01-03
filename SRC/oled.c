/********************* (C) COPYRIGHT 2016 e-Design Co.,Ltd. ********************
 File Name :      olde.c
 Version :        1.7a
 Description:
 Author :         Ning
 Data:            2017/11/22
 History:
 2017/06/20       ������ͬӲ���汾�µĶ�������;
 2017/10/21       �Ż�����iic����;
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
 ������: Set_ShowPos
 ��������:Ҫ��ʾ���ݵ�λ��
 �������:x:������,y:������(0,8,16,24)
 ���ز���:NULL
 *******************************************************************************/
void OLED_Set_Pos(uint8_t x, uint8_t y)
{
	x += 0;
	Write_IIC_Command(0xb0 + y);
	Write_IIC_Command(((x & 0xf0) >> 4) | 0x10);
	Write_IIC_Command(x & 0x0f); //Write_IIC_Command((x&0x0f)|0x01);
}
/*******************************************************************************
 ������: Oled_DrawArea
 ��������:��ʾһ������
 �������: x0:��ʼ������
 y0:��ʼ������(0,8,16,24)
 wide:��ʾ���ݿ��
 high:��ʾ���ݸ߶�
 ptr:��ʾ�����ݿ�ָ��
 ���ز���:��һ��ָ��
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

	//y���ж�����.
	//m�Ƕ��ٸ�8��,ÿ��һ��mֵ��һ.
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
 ������: Init_Oled
 ��������:��ʼ��LED����
 �������:NULL
 ���ز���:NULL
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

