#ifndef _MAIN_H_
#define _MAIN_H_

#include "stm32f10x.h"

#define FULL_SPEED_PWM  2400
#define LOW_SPEED_PWM   0
#define MAX_PWM         100
#define MAX_ANGLE       80      //最大角度范围
#define MAX_TORQUE      6       //最大挡位
#define MAX_ROTATE_I    1800    //最大堵转电流
#define MAX_RUNTIME     12000   //最大持续转动时间  = /120s
#define GEARS_UNIT_I    300     //每档堵转电流差

struct INFO_SYS
{
	uint8_t ver[16];
	uint8_t start_angle;		//开始转动角度
	uint8_t torque_level;		//扭力等级
	uint32_t moto_timecnt;		//电机转动时间
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
	IDLE_MOD = 1,	//空闲
	CTR_MOD,		//控制模式
	SET_MOD,		//设置模式
	LP_MOD,			//LowPower模式
	STANDBY_MOD,	//待机
	SHUT_DOWN,		//其他
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

