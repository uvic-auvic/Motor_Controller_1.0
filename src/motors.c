#include "motors.h"
#include "Timer.h"
//#include "cortexm/ExceptionHandlers.h"
#include "stm32f0xx.h"
#include "stm32f0xx_misc.h"
#include "FreeRTOS.h"
#include "timers.h"

static xTimerHandle xSafetyStopTimer;
static const long timer_id = 0xFAFA;

static void vSafetyStopTimerCallback( TimerHandle_t *pxTimer )
{
	if( (long)pvTimerGetTimerID(pxTimer) == timer_id)
		Motors_Stop();
}

static void software_timer_config(void){
	xSafetyStopTimer = xTimerCreate(
			"SafetyStopTimer",			/* Just a name */
			5000 / portTICK_RATE_MS,	/* Configure timer for 5 seconds */
			pdFALSE,					/* Disable auto-reload */
			(void *)timer_id,			/* Unique timer identifier */
			vSafetyStopTimerCallback);	/* Specify callback function */
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

extern void Motor_PWM(motor m, unsigned int percent_10000){
	//int to uint_8 causes error
	if(percent_10000 == 0){
		percent_10000 = 1;
	}
	switch(m){
	case (motor1):
		TIM3->CCR3 = percent_10000 -1;
		break;
	case (motor2):
		TIM3->CCR2 = percent_10000 -1;
		break;
	case (motor3):
		TIM3->CCR1 = percent_10000 - 1;
		break;
	}
}

extern void Motor_Speed(motor m, unsigned int percent_10000, motor_direction dir){
	int speed = NEUTRAL + dir * percent_10000;
	Motor_PWM(m, speed);

	/* Start timer. If expires, motors will stop */
	xTimerReset(xSafetyStopTimer, 100);
}

extern void Motors_Stop(void){
	Motor_PWM(motor1, NEUTRAL);
	Motor_PWM(motor2, NEUTRAL);
	Motor_PWM(motor3, NEUTRAL);
}

extern void Motors_init(void){
	timer3_it_config();
	software_timer_config();
	Motor_PWM(motor1, NEUTRAL);
	Motor_PWM(motor2, NEUTRAL);
	Motor_PWM(motor3, NEUTRAL);
}
