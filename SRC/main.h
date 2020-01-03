#ifndef _MAIN_H_
#define _MAIN_H_

#include "stm32f10x.h"

#define FULL_SPEED_PWM  2400
#define LOW_SPEED_PWM   0
#define MAX_PWM         100
#define MAX_ANGLE       80      //���Ƕȷ�Χ
#define MAX_TORQUE      6       //���λ
#define MAX_ROTATE_I    1800    //����ת����
#define MAX_RUNTIME     12000   //������ת��ʱ��  = /120s
#define GEARS_UNIT_I    300     //ÿ����ת������

struct INFO_SYS
{
	uint8_t ver[16];
	uint8_t start_angle;		//��ʼת���Ƕ�
	uint8_t torque_level;		//Ť���ȼ�
	uint32_t moto_timecnt;		//���ת��ʱ��
};

enum WORK_STATUS
{
	DMOCONNECT = 1,
	DMOSETGROUP,
	DMOAUTOPOWCONTR,
	DMOSETVOX,
	DMOSETMIC,
	DMOMES ,
	DMOVERQ,
	DMOMESSEND,
};

enum WORK_MOD
{
	IDLE_MOD = 1,	//����
	CTR_MOD,		//����ģʽ
	SET_MOD,		//����ģʽ
	LP_MOD,			//LowPowerģʽ
	STANDBY_MOD,	//����
	SHUT_DOWN,		//����
};

extern struct INFO_SYS info_def;
extern uint16_t The_Current;
extern uint32_t current_limt;

void Motor_Foreward(uint16_t pwm);
void Motor_Reversal(uint16_t pwm);
void Motor_Standby(void);
void Motor_Brake(void);
float Motor_Process(void);
void Idle_Process(void);
void TorqueLv_Set(void);
void Mode_Switching(void);
void Shut_Down(void);
void Stand_By(void);
void Motor(uint8_t dir,uint8_t pwm);

#endif

