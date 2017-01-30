#include "motors.h"
#include "Timer.h"
//#include "cortexm/ExceptionHandlers.h"
#include "stm32f0xx.h"
#include "stm32f0xx_misc.h"

#define SCALE (10000)
#define NEUTRAL (900)
#define FWD_MAX (1200)
#define REV_MAX (600)

extern void Motor_PWM(motor m, unsigned int percent_10000){
	//int to uint_8 causes error
	if(percent_10000 == 0){
		percent_10000 = 1;
	}
	switch(m){
	case (right_side):
		TIM3->CCR1 = percent_10000 - 1;
		break;
	case (left_side):
		TIM3->CCR2 = percent_10000 -1;
		break;
	case(top_right):
		TIM3->CCR3 = percent_10000 -1;
		break;
	case(top_left):
		TIM3->CCR4 = percent_10000 -1;
		break;
	case(middle):
		TIM2->CCR4 = percent_10000 -1;
		break;
	}
}

extern void Motor_Speed(motor m, unsigned int percent_10000, motor_direction dir){
	int speed = NEUTRAL + dir * percent_10000;
	Motor_PWM(m, speed);
}

static void timer3_it_config(void){
	/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	/* Time base configuration */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	TIM_TimeBaseStructure.TIM_Period = 10000 - 1; // 600 kHz down to 60Hz (2 ms)
	TIM_TimeBaseStructure.TIM_Prescaler = 80 - 1; // 48 MHz Clock down to 600 kHz (adjust per your clock)
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	/* TIM IT enable */
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	RCC->AHBENR |= RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB;

	//initialize structs
	GPIO_InitTypeDef GPIO_struct;

	//initialize the LEDs
	GPIO_struct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_struct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_struct.GPIO_OType = GPIO_OType_PP;
	GPIO_struct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_struct.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_Init(GPIOA, &GPIO_struct);

	GPIO_struct.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB, &GPIO_struct);

	TIM_OCInitTypeDef outputChannelInit = {0,};
	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = NEUTRAL - 1; //9%
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM3, &outputChannelInit);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_OC2Init(TIM3, &outputChannelInit);
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_OC3Init(TIM3, &outputChannelInit);
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_1);

	/* TIM3 enable counter */
	TIM_Cmd(TIM3, ENABLE);
}

static void timer2_it_config()
{
	//Enable the interrupt this is needed to flicker an LED A#
	NVIC_InitTypeDef NVIC_InitStructure;
	//Enable Tim2 global interrupt
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	/* Time base configuration */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	TIM_TimeBaseStructure.TIM_Period = 10000 - 1; // 2kHz down to 1Hz (1000 ms)
	TIM_TimeBaseStructure.TIM_Prescaler = 80 - 1; // 48 MHz Clock down to 2kHz (adjust per your clock)
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	/* TIM IT enable */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	TIM_OCInitTypeDef outputChannelInit = {0,};
	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = NEUTRAL - 1; //9%
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC4Init(TIM2, &outputChannelInit);
	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

	//initialize structs
	GPIO_InitTypeDef GPIO_struct;

	//initialize the LEDs
	GPIO_struct.GPIO_Pin = GPIO_Pin_3;
	GPIO_struct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_struct.GPIO_OType = GPIO_OType_PP;
	GPIO_struct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_struct.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_Init(GPIOA, &GPIO_struct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_2);

	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);
}

static void timer16_it_config(){
	//Enable the interrupt this is needed to flicker an LED A#
	NVIC_InitTypeDef NVIC_InitStructure;
	//Enable Tim16 global interrupt
	NVIC_InitStructure.NVIC_IRQChannel=TIM16_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* TIM6 clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
	/* Time base configuration */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	TIM_TimeBaseStructure.TIM_Period = 50000 - 1; // 50kHz down to 1Hz (1000 ms)
	TIM_TimeBaseStructure.TIM_Prescaler = 960 - 1; // 48 MHz Clock down to 50 kHz (adjust per your clock)
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM16, &TIM_TimeBaseStructure);
	/* TIM IT enable */
	TIM_ITConfig(TIM16, TIM_IT_Update, ENABLE);

	/* TIM16 enable counter */
	TIM_Cmd(TIM16, ENABLE);
}

extern void Motors_Stop(void){
	Motor_PWM(right_side, NEUTRAL);
	Motor_PWM(left_side, NEUTRAL);
	Motor_PWM(top_right, NEUTRAL);
	Motor_PWM(top_left, NEUTRAL);
	Motor_PWM(middle, NEUTRAL);
}

extern void Motors_init(void){
	timer3_it_config();
	Motor_PWM(right_side, NEUTRAL);
	Motor_PWM(left_side, NEUTRAL);
	Motor_PWM(top_right, NEUTRAL);
	Motor_PWM(top_left, NEUTRAL);

	timer2_it_config();
	Motor_PWM(middle, NEUTRAL);

	timer16_it_config();
}
