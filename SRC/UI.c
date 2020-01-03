/********************* (C) COPYRIGHT 2016 e-Design Co.,Ltd. ********************
 File Name :      UI.c
 Version :        1.7a
 Description:
 Author :         Ning
 Data:            2017/11/22
 History:
 2017/04/01       ��ʾ��λ��ת���ʹ���ʱ;
 2017/08/01       ��ʾת��ʱ������С;
 2017/8/2         ��ʾ��������
 2017/09/10       ��ʾ���ʱ������;
 *******************************************************************************/
#include <stdio.h>
#include "Oled.h"
#include "font.h"
#include "UI.h"
#include "string.h"
#include "main.h"
#include "hardware.h"
#include "xprintf.h"

uint8_t gScrew_position = 17;		/*��˿ͼ����ʾλ��*/

/*******************************************************************************
 ������: Clear_Screen
 ��������:����
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Clear_Screen(void)
{
	uint8_t data[128];
	uint8_t i;

	memset(&data[0], 0, 128);
	for (i = 0; i < 2; i++)
	{
		Oled_DrawArea(0, i * 8, 128, 8, data);
	}
}
/*******************************************************************************
 ������: Display_Str
 ��������:��ʾ16*16�ַ���
 �������:x: λ�� str :��ʾ�ַ���
 ���ز���:NULL
 *******************************************************************************/
void Display_Str(uint8_t x, uint8_t y, char* str)
{
	uint8_t* ptr;
	uint8_t temp;

	if ((x < 1) || (x > 16))
	{
		x = 0;
	}
	else
	{
		x--;
	}

	while (*str != 0)
	{
		temp = *str++ - ' '; //�õ�ƫ�ƺ��ֵ
		ptr = (uint8_t*) ASCII6x8;
		ptr += temp * 6;
		Oled_DrawArea(x * 8, y, 6, 8, (uint8_t*) ptr);
		x++;
	}
}
/*******************************************************************************
 ������: Show_SetLv
 ��������:��ʾ���õ�λ
 �������:gear ��λ
 ���ز���:NULL
 *******************************************************************************/
void Show_SetLv(uint8_t gear)
{
	uint8_t* ptr;
	ptr = (uint8_t*) SET_CNT + 96 * 2 * gear;

	Oled_DrawArea(0, 0, 96, 16, ptr); //��������Ļ
}
/*******************************************************************************
 ������: Low_Power
 ��������:��ʾ�͵�������
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Low_Power(void)
{
	uint8_t *ptr;
	ptr = (uint8_t*) LOW_POWER;

	Oled_DrawArea(0, 0, 96, 16, ptr);
}
/*******************************************************************************
 ������: Show_BatV
 ��������:��ʾ����|���״̬
 �������:step: ���� Charging:���״̬
 ���ز���:NULL
 *******************************************************************************/
void Show_BatV(uint8_t step, uint8_t Charging)
{
	uint8_t *ptr;
	static uint8_t min_v = 10;

	if (step > 10 && Charging == 0)
	{
		step = 10;
	}

	if (Charging > 0)
	{
		//���ʱ�������������� �����Զ���
		if (step > min_v)
		{
			min_v = step;
		}
	}
	else //���ڳ��ʱ
	{
		if (min_v >= step)
		{
			min_v = step;
		}
		else
		{
			step = min_v; //��ֹ������ʾ����
		}
	}

	if (Charging == 0)
	{
		//���ǳ��
		ptr = (uint8_t*) BAT_STAT + 20 * step;
		Oled_DrawArea(0, 0, 10, 16, ptr);
	}
	else if (Charging == 1)
	{
		//���
		ptr = (uint8_t*) BAT_STAT + 20 * (10 + step);
		Oled_DrawArea(0, 0, 10, 16, ptr);
	}
	else if (Charging == 2)
	{
		ptr = (uint8_t*) BAT_STAT + 20 * 21;
		Oled_DrawArea(0, 0, 10, 16, ptr);
	}
}
/*******************************************************************************
 ������: Clear_Screw
 ��������:����
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Clear_Screw(void)
{
	uint8_t data[128], i;

	memset(&data[0], 0, 128);
	for (i = 0; i < 2; i++)
	{
		Oled_DrawArea(11, i * 8, 81, 8, data);	//92 -16
	}
}
/*******************************************************************************
 ������: Show_ScrewMove
 ��������:��̬��ʾ��˿ͼƬ
 �������:status : Screw down status = 0 down��1up
 ���ز���:NULL
 *******************************************************************************/
void Show_ScrewMove(uint8_t status)	//Screw down status = 0 down��1up
{
	uint8_t *ptr;
	static uint8_t i = 1, step = 0;

	if (gScrew_position != i)
		i = gScrew_position;
	if (status == 0)
	{ //down
		ptr = (uint8_t*) SD_IDF + 64 * step;
		Oled_DrawArea(11 + i, 0, 32, 16, ptr);
		i++;
		if (i >= 50 || i <= 0)
			i = 0; //50-16
	}
	if (status == 1)
	{ //up
		ptr = (uint8_t*) SD_IDF + 64 * step;
		Oled_DrawArea(11 + i, 0, 32, 16, ptr);
		i--;
		if (i <= 0 || i >= 50)
			i = 49; //50-16
	}
	ptr = (uint8_t*) LEVEL_IDF + 11 * 2 * info_def.torque_level;
	Oled_DrawArea(0, 0, 11, 16, ptr); //��λ

	gScrew_position = i;
	step++;
	if (step == 4)
		step = 0;

	/*2017.8.1��ǰ����������*/
	if (step % 2)
	{
		if (The_Current > MAX_ROTATE_I) //������ת����1200
		{
			ptr = (uint8_t*) I_Size + 56; //������С��ʾ��
			Oled_DrawArea(93, 0, 2, 16, ptr);
		}
		else if (The_Current == 0) //0
		{
			ptr = (uint8_t*) I_Size;
			Oled_DrawArea(93, 0, 2, 16, ptr);
		}
		else
		{
			ptr = (uint8_t*) I_Size + (The_Current / 85 * 4);
			Oled_DrawArea(93, 0, 2, 16, ptr);
		}
	}
}
/*******************************************************************************
 ������: Show_Screw
 ��������:�̶�λ����ʾ��˿
 �������:uint8_t mode  0:����ģʽ   1:����ģʽ�µĹ̶���ʾ
 ���ز���:NULL
 *******************************************************************************/
void Show_Screw(uint8_t mode)
{
	uint8_t *ptr;
	if (mode)
	{
		ptr = (uint8_t*) LEVEL_IDF + 11 * 2 * info_def.torque_level;
		Oled_DrawArea(0, 0, 11, 16, ptr); //��λ
		ptr = (uint8_t*) CONSULT + 8;
		Oled_DrawArea(92, 0, 4, 16, ptr); //����
	}
	else
	{
		ptr = (uint8_t*) LEVEL_IDF + 11 * 2 * info_def.torque_level;
		Oled_DrawArea(75, 0, 11, 16, ptr); //��λ
		ptr = (uint8_t*) CONSULT;
		Oled_DrawArea(92, 0, 4, 16, ptr); //����
	}
	if (mode != 2)
	{
		ptr = (uint8_t*) SD_IDF;
		Oled_DrawArea(27, 0, 32, 16, ptr); //��˿
		gScrew_position = 17;
	}
}

