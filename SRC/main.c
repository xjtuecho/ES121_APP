/********************* (C) COPYRIGHT 2016 e-Design Co.,Ltd. ********************
 File Name :      Main.c
 Version :        1.7a
 Description:
 Author :         Ning
 Data:            2017/11/22
 History:
 2017/06/20       ���Ӳ���汾�ж�;
 2017/11/11       ���AD���Ź�;
 *******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Oled.h"
#include "font.h"
#include "Disk.h"
#include "main.h"
#include "hardware.h"
#include "L3G4200D.h"
#include "UI.h"
#include "iic.h"
#include "xprintf.h"

// �汾1.7,����ת��, 1��Ť��
// �޸İ汾����Ҫdisk.c�е�Def_set[]����
struct INFO_SYS info_def = { "1.10", 0, 0, 0 };

enum WORK_MOD gMode = IDLE_MOD;			// ��ʼ״̬
enum WORK_MOD gPre_Mode = IDLE_MOD;		// ǰһ״̬

uint32_t gMoto_timecnt;				//����ת��ʱ��
uint8_t gMoto_cntflag = 0;			//ת����־ 0:ûת��1:ת
uint32_t gCur_timer;				//ʱ���
uint32_t gLast_timer;				//ʱ���
uint16_t The_Current = 0;			//2017.8.1��ǰ����
uint32_t current_limt = 0;			//��ת����

static uint8_t turbo = 0;			//��ѹ��־
static uint8_t Low_vol_sign = 0;	//��ѹ��־

uint16_t interval;					//�����������
uint16_t bat_vol;					//��ص�ѹ����λ10mV
uint16_t bat_value;					//��ص��� 0-10��ʾ100%

void TURBO_OFF(void)
{
	turbo = 0;
	GPIO_ResetBits(GPIOA, GPIO_Pin_10);
}

/*******************************************************************************
 ������: Motor_Foreward
 ��������:�����ת
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Motor_Foreward(uint16_t pwm)
{
	if (SYSCLK_8M == frequency_pos && 0 == info_def.torque_level)
		TIM_SetCompare1(TIM1, pwm * FULL_SPEED_PWM / MAX_PWM / 6);			//��
	else
		TIM_SetCompare1(TIM1, pwm * FULL_SPEED_PWM / MAX_PWM);				//��

	TIM_SetCompare2(TIM1, LOW_SPEED_PWM);					//�淽��ת
}

/*******************************************************************************
 ������: Motor_Reversal
 ��������:�����ת
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Motor_Reversal(uint16_t pwm)
{
	TIM_SetCompare1(TIM1, LOW_SPEED_PWM);					//������ת

	if (SYSCLK_8M == frequency_pos && 0 == info_def.torque_level)
		TIM_SetCompare2(TIM1, pwm * FULL_SPEED_PWM / MAX_PWM / 6);			//��
	else
		TIM_SetCompare2(TIM1, pwm * FULL_SPEED_PWM / MAX_PWM);				//��
}

/*******************************************************************************
 ������: Motor_Standby
 ��������:�������
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Motor_Standby(void)
{
	TIM_SetCompare1(TIM1, LOW_SPEED_PWM);
	TIM_SetCompare2(TIM1, LOW_SPEED_PWM);
}

/*******************************************************************************
 ������: Motor_Brake
 ��������:���ɲ��
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Motor_Brake(void)
{
	TURBO_OFF();
	TIM_SetCompare1(TIM1, FULL_SPEED_PWM);
	TIM_SetCompare2(TIM1, FULL_SPEED_PWM);
}

/*******************************************************************************
 ������: Motor
 ��������:���ת��
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Motor_Control(uint8_t dir, uint8_t pwm)			//ת������
{
	if (dir == 1)
		Motor_Foreward(pwm);					//1,˳ʱ��
	else
		Motor_Reversal(pwm);					//0,��ʱ��
}

/*******************************************************************************
 ������: TorqueLv_Set
 ��������:��λ���� Torque Level Set
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void TorqueLv_Set(void)
{
	if (gTimer[1] > 0)
	{
		if (gKey_state == KEY_V1)
		{
			//������λ����
			info_def.torque_level++;
			if (info_def.torque_level == MAX_TORQUE)
			{
				info_def.torque_level = 0;
			}

			gKey_state = NO_KEY; //�������
		}
		Show_SetLv(info_def.torque_level); //���ڵ�ǰѡ�е�λ
	}

	if (gKey_gap < interval)
	{
		//�а�����λ����ģʽ�˳�ʱ�����
		gTimer[1] = 150;
		gTimer[2] = SHUTDOWN_TIME; //�ػ�ʱ��
	}

	if (gKey_state == (KEY_CN | KEY_V1))
	{
		//���������˳�����ģʽ
		gTimer[1] = 0;
		gTimer[2] = SHUTDOWN_TIME;
	}

	if (gTimer[1] == 0)
	{ //�˳�ǰ���浵λ
		gCalib_flag = 1;
		Config_Analysis(); //���������ļ�
		gCalib_flag = 0;
		gMode = IDLE_MOD;
		Clear_Screen();
		if (info_def.torque_level != 1 && info_def.torque_level != 0)
		{
			//���ݵ�λ�������
			current_limt = (MAX_ROTATE_I - ((info_def.torque_level - 2)) * GEARS_UNIT_I);
		}
		else
		{
			current_limt = 1800;
		}

		Set_CurLimit(current_limt, 0); //adc���Ź�����
	}
}
/*******************************************************************************
 ������: Motor_Process
 ��������:������ƹ���
 �������:cur_x ת���Ƕ�
 ���ز���:ת���ĽǶ�
 *******************************************************************************/
float Motor_Process(void)
{
	uint8_t *ptr;
	static float angle = 0.0; //�Ƕ�
	static uint8_t pre_dir = 0; //previous direction , 0 up ,1 dowm
	static uint8_t run_flag = 0; //ת����־
	static uint8_t pre_pwm = 100; //previous pwm
	static uint16_t last_pwm = 0;
	static int Run_Time = 0; //��ʼת����ʱ��
	static uint8_t Restart_Pos = 1; //���¿�ʼת��־λ
	static uint8_t Stop_Pos = 0; //����ת��ʱ��ֹͣ
	uint16_t limt_cnt;
	float cur_scale;
	float temp_angle;
	cur_scale = Get_Angle();   //��ȡ��ǰת���Ƕ�
	static uint8_t dir_change = 0;   //2017.10.31�ı�ת��������߿�ʼת��

	if (!gKey_Press)
	{
		//��������
		gMode = IDLE_MOD;
		Clear_Screen();
		angle = 0;
		Run_Time = 0;
		Restart_Pos = 0;
		Stop_Pos = 0;
	}
	else if (gKey_Press)
	{
		//��������
		if (Restart_Pos == 0)   //��ʼת����ʱ���������ٴ���
		{
			dir_change = 1;
			Restart_Pos = 1;
			Run_Time = Timer_Counter;
		}
		if (Timer_Counter - Run_Time > MAX_RUNTIME)   //ת����ʱ
		{
			Stop_Pos = 1;
			angle = 0;
			turbo = 0;
			run_flag = 0;
			last_pwm = 0;
			Motor_Brake();   //���ɲ��
			ptr = (uint8_t*) TIMEOUT_16X16;
			Oled_DrawArea(0, 0, 96, 16, ptr);   //��ʱͼƬ
		}
		gTimer[2] = SHUTDOWN_TIME;   //������λ�ػ�ʱ��

		if (gPre_Mode == IDLE_MOD)
		{
			//����״̬
			gPre_Mode = CTR_MOD;   //�������ģʽ
			angle = 0;
			turbo = 0;
			run_flag = 0;
			last_pwm = 0;
			AWD_entry = 0;

			gTimer[0] = 20;  //��ʱ����������ʱ�� || ��תǰ����ѹʱ��
			gTimer[3] = 0;   //���ͣתʱ��
			gTimer[4] = 50;  //����Ƿ�̶�ģʽ�еĳ���ʱ���ж�
			gTimer[5] = 0;   //��ʾˢ�¼��

			if (Stop_Pos)        //��ʱ����
			{
				ptr = (uint8_t*) TIMEOUT_16X16;
				Oled_DrawArea(0, 0, 96, 16, ptr);        //��ʱͼƬ
			}
			else
			{
				Clear_Screw();
				Show_Screw(1);   //��ʾ�̶���˿
			}
		}

		if (info_def.torque_level == 0 || info_def.torque_level == 1)
		{
			if ((cur_scale > -0.069 && cur_scale < 0)
					|| (cur_scale < 0.069 && cur_scale > 0))
				cur_scale = 0; //ȥ����
		}
		else
		{
			if ((cur_scale > -0.169 && cur_scale < 0) || (cur_scale < 0.169 && cur_scale > 0))
			{
				cur_scale = 0; //ȥ����
			}
		}

		angle += cur_scale;			//�Ƕȼ���
		if (angle >= MAX_ANGLE)
		{
			angle = MAX_ANGLE;
		}
		if (angle <= -MAX_ANGLE)
		{
			angle = -MAX_ANGLE;		//���ƽǶȷ�Χ  0~90/-90~0
		}

		if (gMoto_cntflag == 0 && run_flag == 1)
		{
			//��û��ת����ʱ�� ��¼���ת���ۼ�ʱ��ĳ�ʼֵ
			gMoto_cntflag = 1;
			gCur_timer = Timer_Counter; //��ȡ��ǰʱ��
			gLast_timer = gCur_timer;
		}

		if (Stop_Pos == 0)
		{
			//ת��ʱ,С����ʼ�Ƕ�
			if (angle <= info_def.start_angle && angle >= -info_def.start_angle)
			{
				//��ת��
				run_flag = 0;
				Motor_Brake(); //���ɲ��
				Show_Screw(2);
			}
			else
			{
				//ת������
				if (angle > info_def.start_angle)
				{
					//��,˳ʱ��
					if (pre_dir == 0)
						dir_change = 1;
					pre_dir = 1;
					temp_angle = angle;
				}
				else if (angle < -info_def.start_angle)
				{
					//��,��ʱ��
					if (pre_dir == 1)
						dir_change = 1;
					pre_dir = 0;
					temp_angle = -angle;
				}

				if (info_def.torque_level != 0)
				{
					//�̶��ٶ�ģʽPWMΪ����ҹ̶�
					if (dir_change) //���������߸��ķ���
					{
						if (dir_change == 1) //ֻ����һ��
						{
							last_pwm = 0;
							dir_change++;
						}
						Delay_Ms(4);
						last_pwm++;
						if (last_pwm == MAX_PWM)
						{
							dir_change = 0;
						}

						if (pre_dir) //���������
						{
							current_limt = MAX_ROTATE_I; //��תĬ������ת����
							Set_CurLimit(current_limt, 0); //adc���Ź�����
						}
						else //�淽�����
						{
							if (info_def.torque_level != 1)
							{
								current_limt = (MAX_ROTATE_I - ((info_def.torque_level - 2)) * GEARS_UNIT_I);
							}
							else
							{
								if (temp_angle >= 60)
									current_limt = 1800;
								else
									current_limt = (uint32_t) (temp_angle * 17 + 750);
							}
							Set_CurLimit(current_limt, 0); //adc���Ź�����
						}
					}
					else //û�ı䷽��
					{
						last_pwm = MAX_PWM;
					}
				}
				else
				{
					//A���ٶȿɱ�PWM��Ƕȱ仯���仯
					if (dir_change) //���������߸��ķ���
					{
						if (dir_change == 1) //ֻ����һ��
						{
							last_pwm = 0;
							dir_change++;
						}
						Delay_Ms(4);
						last_pwm++;
						if (last_pwm == 15 + (int) ((temp_angle / 45) * (MAX_PWM - 15)))
						{
							dir_change = 0;
						}

						if (pre_dir)
						{
							current_limt = MAX_ROTATE_I; //��������뷴תĬ������ת����
						}
						else
						{
							current_limt = MAX_ROTATE_I; //�淽�����
						}

						Set_CurLimit(current_limt, 0); //adc���Ź�����
					}
					else //û�ı䷽��
					{
						if (temp_angle >= 45)
						{
							last_pwm = MAX_PWM; //���
						}
						else
						{
							last_pwm = 15 + (int) ((temp_angle / 45) * (MAX_PWM - 15));
						}
					}
				}
				if (!run_flag)
				{
					turbo = 0;
					gTimer[0] = 20; //����ѹʱ��
					run_flag = 1;
				}
				pre_pwm = last_pwm;
			}
		}

		//��ת���������ۼ�
		The_Current = (60000 * Get_Adc(VCUR) / 4096); //��ǰ����  PA0

		bat_vol = (600 * Get_Adc(VBTY)) / 4096; //��ص�ѹ
		bat_vol = (bat_vol * 610) / 600; //��ѹ����

		Low_vol_sign = (bat_vol < 330) ? 1 : 0;

		if (run_flag == 1 && Stop_Pos == 0)
		{
			//ת��ʱ
			if (gTimer[0] == 0 && turbo == 0)
			{
				turbo = 1;
				if (Low_vol_sign == 1)
				{
					turbo = 0;
				}
				else
				{
					//�����ѹ
					TURBO_ON();
				}
			}

			Motor_Control(pre_dir, pre_pwm);

			bat_vol = (600 * Get_Adc(VBTY)) / 4096; //��ص�ѹ
			bat_vol = (bat_vol * 610) / 600; //��ѹ����

			Low_vol_sign =  (bat_vol < 330) ? 1 : 0;

			if (gTimer[5] == 0) // && turbo)
			{
			//	char str[4][8];
				Clear_Screw(); //����
				//������ת������ʾ��˿���·���
				Show_ScrewMove(pre_dir); //��̬��ʾ��˿ͼƬ
			//	xsprintf(str[0], "%d", pre_dir);
			//	Display_Str(3,0,str[0]);
			//	xsprintf(str[1], "%d", pre_pwm);
			//	Display_Str(3,5,str[1]);
			//	xsprintf(str[2], "%d", Low_vol_sign);
			//	Display_Str(8,0,str[2]);
			//	xsprintf(str[3], "%d", turbo);
			//	Display_Str(8,5,str[3]);
				gTimer[5] = 5;
			}
		}
		else
		{
			return angle;
		}

		//��ת���������ۼ�
		The_Current = (60000 * Get_Adc(VCUR) / 4096); //��ǰ����  PA0

		if (info_def.torque_level != 0)
		{
			if (info_def.torque_level == 1)
			{
				if (temp_angle >= 60)
					limt_cnt = 250;
				if (temp_angle >= 40 && temp_angle < 60)
					limt_cnt = 200;
				if (temp_angle >= 20 && temp_angle < 40)
					limt_cnt = 150;
				if (temp_angle >= 0 && temp_angle < 20)
					limt_cnt = 100;
			}
			else
			{
				//������λ��������
				limt_cnt = (500 - (info_def.torque_level - 2) * 100) / 2;
			}
		}
		else
			limt_cnt = 500 / 2; //A������ֵ

		if ((gTimer[0] == 0) && (AWD_entry > limt_cnt))
		{
			//��ת����,���ɲ��,3s�ڲ�ת�������ת�������200ms����ѹ��
			GPIO_ResetBits(GPIOB, GPIO_Pin_2);
			Motor_Brake();	//���ɲ��
			AWD_entry = 0;
			turbo = 0;	//2017.12.18��ת��3����ѹ
			gMode = STANDBY_MOD;
			gTimer[3] = 300;
			gTimer[0] = 320;
		}
	}
	return angle;
}
/*******************************************************************************
 ������: Idle_Process
 ��������:����״̬����ת��
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Idle_Process(void)
{
	static uint8_t LowPwer_cnt = 0;
	unsigned long dt = 0;

	if (gPre_Mode != IDLE_MOD)
	{
		gPre_Mode = IDLE_MOD;
	}

	Motor_Brake();	//���ɲ��

	if (gMoto_cntflag == 1)
	{
		//��һ���ɿ�����,������ת��ʱ��
		gCur_timer = Timer_Counter; //��ȡ��ǰʱ��
		dt += Elapse_Value(gLast_timer, gCur_timer); //ÿ��λ10 ms
		gMoto_timecnt += dt;

		gMoto_cntflag = 0;
		info_def.moto_timecnt = gMoto_timecnt / 100; //��
		Write_MtFlash(); //������ת��ʱ��
	}

	bat_vol = (600 * Get_Adc(VBTY)) / 4096; //��ص�ѹ
	bat_vol = (bat_vol * 610) / 600; //��ѹ����

	// ������ʾ����
	if (gTimer[5] == 0)
	{
		// ��ʾ��ѹ�Լ���˿�Ĵ�������

		// 3.95V���ϵ���1
		// 3.65~3.95֮�����Լ���
		// 3.2~3.65V ֮�����Ϊ0.1
		// 3.2V ���µ���Ϊ0
		if (bat_vol >= 395)
		{
			bat_value = (bat_vol >= 405) ? 10 : 9;
		}

		if (bat_vol < 395 && bat_vol > 365)
		{
			bat_value = (bat_vol-366)/4;
			bat_value += 2;
		}

		if (bat_vol <= 365)
		{
			bat_value = (bat_vol > 320) ? 1 : 0;
		}

		if (bat_vol >= 415)
		{
			bat_value++;
		}

		if (bat_vol < 330)
		{
			Low_vol_sign = 1;
		}

		if (bat_vol > 340)
		{
			Low_vol_sign = 0;
		}

		if (Get_Adc(VIN) < 500) //û����USB
		{
			//no usb ,only have bat.
			if (SYSCLK_48M == frequency_pos)
			{
				USB_Port(DISABLE);
				frequency_pos = SYSCLK_8M;
				RCC_Config();
				Delay_Init();
				Init_Timer2();
				PWM_Init(400, 0, 0);
				interval = 300;
			}

			if (bat_vol < 320)
			{
				bat_vol = 320;
			}

			Show_BatV(bat_value, 0); //��ʾ����|���״̬
		}
		else //����USB
		{
			if (SYSCLK_8M == frequency_pos)
			{
				USB_Port(DISABLE);
				frequency_pos = SYSCLK_48M;
				RCC_Config();
				Delay_Init();
				Init_Timer2();
				PWM_Init(2400, 0, 0);
				interval = 50;
				Delay_Ms(200);
				USB_Port(ENABLE);
				USB_Init();
			}

			//��ʾ����|���״̬
			if (READ_CHG == 0)
			{
				Show_BatV(bat_value, 1);
			}
			else
			{
				Show_BatV(bat_value, 2);
			}
		}
		Show_Screw(0); //�̶�λ����ʾ��˿
		gTimer[5] = 50;
	}

	if (keypress_cnt == 3 && (gMode != LP_MOD))
	{
		//�������ΰ�����������ģʽ
		gMode = SET_MOD;
		keypress_cnt = 0; //������������
		if (gTimer[1] > 150)
		{
			gTimer[1] = 150;
		}
		gKey_state = NO_KEY;
		Clear_Screen();
	}

	if (bat_vol < 320)
		LowPwer_cnt++; 	//�͵�ѹ�ۼƼ���,��ֹ��ʱ���ת���͵�ѹ,
	else
		LowPwer_cnt = 0;

	if (LowPwer_cnt > 150 && (Get_Adc(VIN) < 500) && (!gKey_Press))
	{
		//�͵�ѹ����LowPowerģʽ
		gMode = LP_MOD;
		gTimer[2] = 500; //û��USB�͵�ѹ,5���ػ�.
		Motor_Brake(); //���ɲ��
		Clear_Screen();
	}
}
/*******************************************************************************
 ������: Shut_Down
 ��������:�ػ�����
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Shut_Down(void)
{
	gCalib_flag = 1;
	Config_Analysis();         //����moto_timecont��config.txt
	gCalib_flag = 0;
	Clear_Screen();

	if (HWVER_1P3 == version_number)
	{
		POWER_OFF_1();		//PD0
	}
	else
	{
		POWER_OFF();		//���͹ػ�PB4
	}
}
/*******************************************************************************
 ������: Stand_By
 ��������:��������
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Stand_By(void)
{
	if (gTimer[3] == 0 || (!gKey_Press))
	{
		gMode = CTR_MOD;
	}
}

/*******************************************************************************
 ������: Mode_Switching
 ��������:ģʽ����
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Mode_Switching(void)
{
	switch (gMode)
	{
		//ģʽת��
	case IDLE_MOD:				//����ģʽ(��ʼ״̬)
		Adc_Init();				//��ʼ��ADC
		Idle_Process();			//����״̬����ת��
		if (gKey_Press)
		{
			Adc_Init();			//��ʼ��ADC
			Get_Angle();		//ת�������ĽǶ�
			gMode = CTR_MOD;	//�������ģʽ
		}
		if (gTimer[2] == 0 && Get_Adc(VIN) < 500)
		{
			// �ػ�ʱ��Ϊ0��û��USB����ʱ,Shut Down
			gMode = SHUT_DOWN;   //����ػ�
		}
		break;

	case CTR_MOD:   //����ģʽ
		Motor_Process();   //������ƹ���
		break;

	case STANDBY_MOD:   //����ģʽ
		Stand_By();
		break;

	case SET_MOD:   //����ģʽ
		TorqueLv_Set();   //��λ����
		break;

	case LP_MOD:    //�͵�ѹģʽ
		Low_Power(); //��ʾ�͵�ѹ����
		if (Get_Adc(VIN) > 500)
		{
			//��USB�����򷵻ؿ���ģʽ
			gMode = IDLE_MOD;
			Clear_Screen();
		}
		else
		{ //�ػ�ʱ��Ϊ0��û��USB����ʱ,Shut Down
			if (gTimer[2] == 0)
			{
				gMode = SHUT_DOWN;
			}
		}
		break;

	case SHUT_DOWN:
		Shut_Down(); //�ػ�
		break;

	default:
		break;
	}
}


/*******************************************************************************
 ������: main
 ��������:������
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
int main(void)
{
	RCC_Config(); //ʱ�ӳ�ʼ��
	Delay_Init(); //��ʼ���ӳٺ���
	GPIO_Config();
	NVIC_Configuration(); //NVIC��ʼ��
	Adc_Init();
	Delay_Ms(10);

	if (Get_Adc(VIN) > 500)
	{
		//��USB
		USB_Port(DISABLE);
		Delay_Ms(200);
		USB_Port(ENABLE);
		USB_Init();
	}

	Disk_BuffInit(); //U�����ݶ�ȡ
	Config_Analysis(); //��������U��
	Init_L3G4200D(); //��ʼ��L3G4200D
	Init_Timer2(); //��ʼ����ʱ��2
	PWM_Init(2400, 0, 0); //20k
	Init_Oled(); //��ʼ��OLED
	Clear_Screen();
	Read_MtCnt(); //��ȡ���ת��ʱ��

	Init_Gtime(); //��ʼ��Gtime[]
	gKey_state = NO_KEY;
	Start_Watchdog(3000);

	version_number = Hardware_version();
	if(HWVER_1P3 == version_number)
	{
		// ���³�ʼ��
		Version_Modify();
	}

	if (info_def.torque_level != 1 && info_def.torque_level != 0)
	{
		current_limt = (MAX_ROTATE_I - ((info_def.torque_level - 2)) * GEARS_UNIT_I); //���ݵ�λ�������
	}
	else
	{
		current_limt = 1800;
	}

	Set_CurLimit(current_limt, 0); //adc���Ź�����

	while (1)
	{
		KickDog();
		Mode_Switching();
	}
}

