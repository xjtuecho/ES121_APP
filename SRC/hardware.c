/********************* (C) COPYRIGHT 2016 e-Design Co.,Ltd. ********************
 File Name :      Hardware.c
 Version :        1.7a
 Description:
 Author :         Ning
 Data:            2017/11/22
 History:
 2017/06/20       ����Ӳ���汾�жϹ���;
 *******************************************************************************/
#include "hardware.h"
#include "main.h"
#include "Oled.h"
#include "stdio.h"
#include "Disk.h"
#include "L3G4200D.h"
#include "iic.h"

uint32_t gKey_state;
uint32_t gKey_gap;
volatile uint32_t gMs_timeout;
// 10ms �ݼ�������
// 0: ����ѹ?
// 1: ����
// 2: �ػ�Shutdown
// 3: ����
// 4: �̶�ģʽ�еĳ���ʱ�䣬ʵ��δʹ��
// 5: OLED��ʾ��50msˢ��һ��
volatile uint32_t gTimer[7];
volatile uint32_t Timer_Counter; //��ʱ��2�жϼ���10ms,��Ϊ�������ʱ����
volatile uint8_t gKey_Press = 0;
uint8_t keypress_cnt = 0;

#define SI_DATA		25

uint16_t ADC1ConvertedValue[3];
uint32_t adc_avg[3], adc_count[3];
uint16_t AWD_entry, ch_num;

uint8_t frequency_pos  = SYSCLK_48M;		// ϵͳʱ��     0=8MHz 1=48MHz
uint8_t version_number = HWVER_1P4;			// �汾��      0=v1.4 1=v1.3

/*******************************************************************************
 ������: NVIC_Configuration
 ��������:NVIC��ʼ��
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig (NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);          // Enable the DMA Interrupt

	/* Configure and enable ADC interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
 ������: RCC_Config
 ��������:ʱ�ӳ�ʼ��
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void RCC_Config(void)
{
	uint8_t RCC_Getclk = 0x00;

	RCC_DeInit();

	FLASH_PrefetchBufferCmd (FLASH_PrefetchBuffer_Enable);

	if (SYSCLK_48M == frequency_pos)
	{
		FLASH_SetLatency (FLASH_Latency_1);   // Flash 1 wait state for 48MHz
		RCC_Getclk = 0x08;
	}
	else
	{
		FLASH_SetLatency (FLASH_Latency_0);   // 8MHz
		RCC_Getclk = 0x00;
	}

	RCC_CFGR_CFG();
	RCC_PLL_EN();
	RCC_HSICmd(ENABLE);
	RCC_PLLCmd(ENABLE);
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
	{
	}

	RCC_SYSCLKConfig((SYSCLK_48M == frequency_pos) ? RCC_SYSCLKSource_PLLCLK : RCC_SYSCLKSource_HSI);

	while (RCC_GetSYSCLKSource() != RCC_Getclk)
	{
	}

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SRAM | RCC_AHBPeriph_DMA1, ENABLE);

	if (SYSCLK_48M == frequency_pos)
	{
		RCC_USBCLKConfig (RCC_USBCLKSource_PLLCLK_Div1);       // USBCLK = 48MHz
	}

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   	//ʹ��DMAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//ʱ��ʹ��(�򿪵�Դ)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_ADC1
		| RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD
		| RCC_APB2Periph_AFIO, ENABLE);

//	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M
}

/*******************************************************************************
 ������: Init_GTIME
 ��������:��ʼ����ʱ��
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Init_Gtime(void)
{
	uint8_t i;

	for (i = 0; i < 6; i++)
	{
		gTimer[i] = 0;
	}

	gTimer[2] = SHUTDOWN_TIME;       //�ػ�ʱ��
}
/*******************************************************************************
 ������: USB_Port
 ��������:���� USB �豸 IO �˿�
 �������:State = ENABLE / DISABLE
 ���ز���:NULL
 *******************************************************************************/
void USB_Port(uint8_t state)
{
	USB_DN_LOW();
	USB_DP_LOW();

	if (state == DISABLE)
	{
		USB_DN_OUT();
		USB_DP_OUT();
	}
	else
	{
		USB_DN_EN();
		USB_DP_EN();
	}
}

/*******************************************************************************
 ������:   Elapse_Value
 ��������: �ӿ�ʼʱ�̵���ʱ������ʱ��
 �������: start ��ʼʱ�䣬curĿǰʱ�䡣
 ���ز���: ������ʱ�䡣
 *******************************************************************************/
uint32_t Elapse_Value(uint32_t start, uint32_t cur)
{
	// ���ݳ�ʼֵ��������ʱ�䡣�� ms Ϊ��λ
	if (cur < start)
	{
		// ��ʱ���Ѿ��ع�
		return 0xffffffff - start + cur;
	}
	else
	{
		return cur - start;
	}
}
/*******************************************************************************
 ������: GPIO_Config
 ��������:GPIO��ʼ��
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);

	//PA0 Vin  ADC1 channel0
	//PA2 Vbty ADC1 channel2
	//PA3 Vcur ADC1 channel3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// PB0 OLED Rst
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;       //OLED Rst
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// PB2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_2);

	// PB4 ���ػ� VE
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	POWER_ON();

	// PA8 PA9 = InA InB PA8 TIM1_CH1,PA9 TIM1_CH2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// PA10 Pump
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	TURBO_OFF();

	// PB7 VK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// PB6 Chrg Chrg  ���0 �����1 ����1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// PB3 = SDA V1.4
	GPIO_InitStructure.GPIO_Pin = SDA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// PA15 = SCL V1.4
	GPIO_InitStructure.GPIO_Pin = SCL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
/*******************************************************************************
 ������: GPIO_Config_1
 ��������:1.3Ӳ���汾��GPIO��ʼ��
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void GPIO_Config_1(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);

	//PA0 Vin ADC1 channel0
	//PA2 Vbty ADC1 channel2
	//PA3 Vcur ADC1 channel3ת��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;       //ncr
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  //ldo_on
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_SetBits(GPIOD, GPIO_Pin_0);

	// PA8 PA9 = InA InB
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// PA10 Pump
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;  //Pump
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	TURBO_OFF();

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;  //k1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;  //Chrg  ���0 �����1 ����1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}

//  PWM Ƶ�ʼ��㹫ʽ:F = (��ʱ��Ƶ��(48M))/((arr + 1)*(psc + 1)),   TIM_Period = arr;
void PWM_Init(uint16_t arr, uint16_t psc, uint8_t version)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_OCStructInit(&TIM_OCInitStructure);

	TIM_TimeBaseStructure.TIM_Period = arr - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  //TIM_CKD_DIV1
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/*
	 PWMģʽ1(TIM_OCMode_PWM1)�� �����ϼ���ʱ��һ��TIMx_CNT<TIMx_CCR1ʱͨ��1Ϊ��Ч��ƽ������Ϊ
	 ��Ч��ƽ�������¼���ʱ��һ��TIMx_CNT>TIMx_CCR1ʱͨ��1Ϊ��Ч��ƽ(OC1REF=0)����
	 ��Ϊ��Ч��ƽ(OC1REF=1)��
	 PWMģʽ2(TIM_OCMode_PWM2)�� �����ϼ���ʱ��һ��TIMx_CNT<TIMx_CCR1ʱͨ��1Ϊ��Ч��ƽ������Ϊ
	 ��Ч��ƽ�������¼���ʱ��һ��TIMx_CNT>TIMx_CCR1ʱͨ��1Ϊ��Ч��ƽ������Ϊ��Ч��
	 ƽ��
	 */

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;

	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM1, ENABLE);
	TIM_Cmd(TIM1, ENABLE);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}
/*******************************************************************************
 ������: Init_Timer2
 ��������:��ʼ����ʱ��2 ʱ���ж�
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Init_Timer2(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;  //�жϽṹ��
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  //��ʱ���ṹ��

	// (48MHz)/48 = 1MHz
	TIM_TimeBaseStructure.TIM_Prescaler = (SYSCLK_48M == frequency_pos) ? (48-1) : (8-1);
	TIM_TimeBaseStructure.TIM_Period = 10000 - 1;	// Interrupt per 10mS �´λ����
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//ʱ��ָ�
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		//TIM���ϼ���ģʽ

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);		//���ݽṹ���ʼ��TIM2
	TIM_ARRPreloadConfig(TIM2, ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);		//ʹ��TIM2���ж�Դ
	TIM_Cmd(TIM2, ENABLE);		//ʹ��TIM(�򿪿���)

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//ʹ��
	NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
 ������: KeyRead
 ��������:��ȡ����ֵ
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Key_Read(void)
{
	static uint16_t key_cnt = 0;
	static uint16_t kgap = 0; //key gap ���ΰ����Ŀ�϶
	static uint16_t press_key = 0;
	uint16_t read_data = 0;

	if (version_number == 0)
	{
		read_data = KEY_READ;  //PB7
	}
	else
	{
		read_data = KEY_READ_1;
	}

	press_key = read_data;  //����

	if (version_number)
	{
		if (press_key == 0x0002)
		{
			kgap = 0;
			key_cnt++;
		}
	}
	else
	{
		if (press_key == 0x0080)
		{
			kgap = 0;
			key_cnt++;
		}
	}

	if (key_cnt >= SI_DATA)
	{
		gKey_state = KEY_CN | KEY_V1;  //����Ϊ����
	}

	if (press_key == 0)
	{
		if (key_cnt < SI_DATA && key_cnt != 0)
		{
			gKey_state = KEY_V1;  //���ð���
			if (kgap < 10)
			{
				keypress_cnt++;  //10*20ms
			}
		}
		kgap++;
		if (kgap >= 10)
		{
			keypress_cnt = 0;  //�����������
		}
		key_cnt = 0;
	}
	gKey_gap = kgap * 20;  //���ð���ʱ����
}

/*******************************************************************************
 ������: Start_Watchdog
 ��������:��ʼ�����Ź�
 �������:ms ���Ź�����
 ���ز���:����1
 *******************************************************************************/
uint32_t Start_Watchdog(uint32_t ms)
{
	IWDG_WriteAccessCmd (IWDG_WriteAccess_Enable);

	/*IWDG counter clock: 40KHz(LSI)/ 32 = 1.25 KHz(min:0.8ms -- max:3276.8ms */
	IWDG_SetPrescaler (IWDG_Prescaler_32);

	/* Set counter reload value to XXms */
	IWDG_SetReload(ms * 10 / 8);

	/* Reload IWDG counter */
	IWDG_ReloadCounter();

	/* Enable IWDG (the LSI oscillator will be enabled by hardware) */
	IWDG_Enable();
	return 1;
}

/*******************************************************************************
 ������: version_Init
 ��������:1.3�汾�����޸�
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Version_Modify(void)
{
	RCC_Config();
	Delay_Init();
	GPIO_Config_1();
	NVIC_Configuration();
	Adc_Init();

	if (Get_Adc(VIN) > 500)
	{
		//��USB
		USB_Port(DISABLE);
		Delay_Ms(200);
		USB_Port(ENABLE);
		USB_Init();
	}

	Disk_BuffInit();			//U�����ݶ�ȡ
	Config_Analysis();			//��������U��
	Init_L3G4200D();			//��ʼ��L3G4200D
	Init_Timer2();
	PWM_Init(2400, 0, 1);		//20k
	Init_Oled();				//��ʼ��OLED
	Clear_Screen();
	Read_MtCnt();				//��ȡ���ת��ʱ��
	Init_Gtime();				//��ʼ��Gtime[]
	gKey_state = NO_KEY;
	Start_Watchdog(3000);
}

/*******************************************************************************
 ������: Adc_Init
 ��������:��ʼ��ADC
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Adc_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	ADC_DeInit(ADC1);  //ADC1��ȫ���Ĵ�������Ϊȱʡֵ

	ch_num = gKey_Press ? 3 : 2;

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  //ģ��ת������������ת��ģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;  //ģ��ת�������ڵ�ͨ��ģʽ  ɨ��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  //ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = ch_num;  //˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���

	if (gKey_Press)
	{
		ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_239Cycles5);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_239Cycles5);	//28 or 55   2017.8.30ͨ��7��Ϊ4
	}
	else
	{
		ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_239Cycles5);
	}

	ADC_DMACmd(ADC1, ENABLE);

	ADC_Cmd(ADC1, ENABLE);						//ʹ��ָ����ADC1

	ADC_ResetCalibration(ADC1);					//ʹ�ܸ�λУ׼

	//�ȴ���λУ׼����
	while (ADC_GetResetCalibrationStatus(ADC1));

	ADC_StartCalibration(ADC1);					//����ADУ׼

	//�ȴ�У׼����
	while (ADC_GetCalibrationStatus(ADC1));

	/* DMA1 channel1 configuration -------------------------------------------*/
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) ADC1ConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = ch_num;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE); //ʹ��DMA��������ж�

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/*******************************************************************************
 ������: Set_CurLimit
 ��������:ADC���Ź�
 �������:����������
 ���ز���:NULL
 *******************************************************************************/
void Set_CurLimit(uint16_t high_ma, uint16_t low_ma)
{
	// Configure high and low analog watchdog thresholds
	ADC_AnalogWatchdogThresholdsConfig(ADC1, high_ma * 4096 / 60000, low_ma * 4096 / 60000);

	// Configure channel14 as the single analog watchdog guarded channel
	ADC_AnalogWatchdogSingleChannelConfig(ADC1, ADC_Channel_3);

	// Enable analog watchdog on one regular channel
	ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_SingleRegEnable);

	// Enable AWD interupt
	ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);
}

/*******************************************************************************
 ������:filter
 ��������:����ADCƽ��ֵ
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void ADC_filter(void)
{
	uint16_t N, j, i;

	if (gKey_Press)
	{
		N = 16, j = 3;
	}
	else
	{
		N = 1, j = 2;
	}
	static uint8_t FirstEntry = 1;
	if (FirstEntry)
	{
		for (i = 0; i < j; i++)
			adc_count[i] = ADC1ConvertedValue[i] * N;
		FirstEntry = 0;
	}

	for (i = 0; i < j; i++)
	{
		adc_count[i] = adc_count[i] - adc_count[i] / N + ADC1ConvertedValue[i];
		adc_avg[i] = adc_count[i] / N;
	}
}

/*******************************************************************************
 ������:Get_Adc
 ��������:��ȡADֵ
 �������:ͨ��:channel
 ���ز���:ADֵ:adc_avg[channel]
 *******************************************************************************/
uint16_t Get_Adc(uint8_t channel)
{
	return adc_avg[channel];
}

/*******************************************************************************
 ������: Hardware_version
 ��������:Ӳ���汾�ж�
 �������:��
 *******************************************************************************/
uint8_t Hardware_version(void)
{
	uint8_t read;
	read = Single_Read(L3G4200_Addr, CTRL_REG1);

	// PD=1 Zen=1 Yen=1 Xen=1
	return (0x0F == read) ? HWVER_1P4 : HWVER_1P3;
}

static uint8_t fac_us = 0; //us��ʱ������
static uint16_t fac_ms = 0; //ms��ʱ������
/*******************************************************************************
 ������: Delay_Init
 ��������:��ʼ���ӳٺ���
 �������:NULL
 ���ز���:NULL
 *******************************************************************************/
void Delay_Init(void)
{
	//ѡ���ⲿʱ��  HCLK/8
	SysTick_CLKSourceConfig (SysTick_CLKSource_HCLK_Div8);
	fac_us = (SYSCLK_48M == frequency_pos) ? 48/8 : 8/8;
	fac_ms = (uint16_t) fac_us * 1000;
}
/*******************************************************************************
 ������: Delay_Ms
 ��������:�ӳٺ���
 �������:��ʱms ����
 ���ز���:NULL
 *******************************************************************************/
void Delay_Ms(uint16_t nms)
{
	uint32_t temp;

	SysTick->LOAD = (uint32_t) nms * fac_ms;	//ʱ�����(SysTick->LOADΪ24bit)
	SysTick->VAL = 0x00;           //��ռ�����
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;          //��ʼ����
	do
	{
		temp = SysTick->CTRL;
	} while (temp & 0x01 && !(temp & (1 << 16)));          //�ȴ�ʱ�䵽��
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;       //�رռ�����
	SysTick->VAL = 0X00;       //��ռ�����
}

