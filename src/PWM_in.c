#include "PWM_in.h"
#include "Timer.h"
///#include "cortexm/ExceptionHandlers.h"
#include "stm32f0xx.h"
#include "stm32f0xx_misc.h"
#include "motors.h"
#include <stdbool.h>

//This code has a lot of Local global variables
//This is necessary because we are using interrupts to get the code running for pwm for some of the channels

unsigned int left_motor_last_pulse;
unsigned int top_right_motor_last_pulse;
unsigned int top_left_motor_last_pulse;
unsigned int middle_motor_last_pulse;

unsigned int left_motor_delta_betw_pulse;
unsigned int top_right_motor_delta_betw_pulse;
unsigned int top_left_motor_delta_betw_pulse;
unsigned int middle_motor_delta_betw_pulse;

unsigned int left_motor_pulse_before_tim_interupt;
unsigned int top_right_motor_pulse_before_tim_interupt;
unsigned int top_left_motor_pulse_before_tim_interupt;
unsigned int middle_motor_pulse_before_tim_interupt;

extern void reset_tim_interrupt_status(){
	left_motor_pulse_before_tim_interupt = false;
	top_right_motor_pulse_before_tim_interupt = false;
	top_left_motor_pulse_before_tim_interupt = false;
	middle_motor_pulse_before_tim_interupt = false;
}

extern void motor_pulse(motor m){
	int result = 0;
	switch(m){
	case left_side:
		if(TIM16->CNT < left_motor_last_pulse)
			result = (CLOCK_PERIOD - left_motor_last_pulse) + TIM16->CNT;
		else
			result = TIM16->CNT - left_motor_last_pulse;
		left_motor_last_pulse = TIM16->CNT;
		left_motor_pulse_before_tim_interupt = true;
		break;
	}
}

extern int read_revolutions(motor m){
	int total_clock_cycles = 0;
	switch(m){
	case right_side:
		total_clock_cycles = TIM1->CCR1;
		break;
	}
	return CLOCK_PERIOD / total_clock_cycles ; // /CYCLES_PER_REV; //Need to fix this after debug
}

// Should return duty cycle of motor PWM
extern int read_duty_cycle(motor m){
	int total_clock_cycles = 0;
	int high_clock_cycles = 0;
	switch(m){
	case right_side:
		total_clock_cycles = TIM1->CCR1;
		high_clock_cycles = TIM1->CCR2;
		break;
	case left_side:
		total_clock_cycles = TIM2->CCR1;
		high_clock_cycles = TIM2->CCR2;
		break;
	case top_right:
		total_clock_cycles = TIM15->CCR1;
		total_clock_cycles = TIM15->CCR2;
		break;
	}
	return ((high_clock_cycles * 10000) / total_clock_cycles); //The hundred is how percent work
}

// Should return PWM frequency
extern int read_frequency(motor m){
	int period_clk_cycles = 0;
	switch(m){
	case right_side:
		period_clk_cycles = TIM1->CCR1;
		break;
	case left_side:
		period_clk_cycles = TIM2->CCR1;
		break;
	case top_right:
		period_clk_cycles = TIM15->CCR1;
		break;
	}
	return CLOCK_PERIOD / period_clk_cycles;
}

static void timer2_IT_config(){
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	/* GPIOB clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* GPIOB Configuration: TIM2 CH4 (PB11) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // Input/Output controlled by peripheral
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	/* Connect TIM2 pins to AF */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_2);

	// Initialize TIM2
	TIM_TimeBaseStructure.TIM_Period = CLOCK_PERIOD-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 960-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	// Set CC1S to 01 and CC2S to 10
	TIM2->CCMR1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1;
	TIM2->CCMR1 &= ~TIM_CCMR1_CC1S_1 & ~TIM_CCMR1_CC2S_0;

	// Enable capture compare on 1&2, and make 1 rising edge, and 2 falling edge
	TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E |TIM_CCER_CC2P;
	TIM2->CCER &= ~TIM_CCER_CC1P & ~TIM_CCER_CC1NP & ~TIM_CCER_CC2NP;

	// TS is used to select valid trigger input (101)
	// SMS is used to put the timer in slave mode with reset (100)
	TIM2->SMCR |= TIM_SMCR_TS_0 | TIM_SMCR_TS_2 | TIM_SMCR_SMS_2;
	TIM2->SMCR &= ~TIM_SMCR_TS_1 & ~TIM_SMCR_SMS_1 & ~TIM_SMCR_SMS_0;
}

static void timer15_IT_config(){
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	/* GPIOB clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	/* TIM15 clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);

	/* GPIOB Configuration: TIM15 CH1 (PB14) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // Input/Output controlled by peripheral
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	/* Connect TIM15 pins to AF */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_2);

	// Initialize TIM15
	TIM_TimeBaseStructure.TIM_Period = 50000-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 960-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM15, &TIM_TimeBaseStructure);

	// Set CC1S to 01 and CC2S to 10
	// Set the input filtering to every 8 clock cycles
	TIM15->CCMR1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1 | TIM_CCMR1_IC1F_0 | TIM_CCMR1_IC1F_1;; // Set bits

	// Enable capture compare on 1&2, and make CC1 rising edge, and CC2 falling edge
	TIM15->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E |TIM_CCER_CC2P; // Set bits

	TIM15->DIER |= TIM_DIER_CC1IE;

	// TS is used to select valid trigger input (101)
	// SMS is used to put the timer in slave mode with reset (100)
	TIM15->SMCR |= TIM_SMCR_TS_0 | TIM_SMCR_TS_2 | TIM_SMCR_SMS_2; // Set bits

	// Enable the counter
	TIM15->CR1 |= TIM_CR1_CEN;
}

static void timer1_IT_config(){
	  GPIO_InitTypeDef GPIO_InitStructure;
	  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	  /* GPIOA clock enable */
	  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	  /* TIM1 clock enable */
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	  /* GPIOA Configuration: TIM1 CH1 (PA8) */
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // Input/Output controlled by peripheral
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // Button to ground expectation
	  GPIO_Init(GPIOA, &GPIO_InitStructure);

	    /* Connect TIM1 pins to AF */
	  GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_2);

	  TIM_TimeBaseStructure.TIM_Period = 50000-1;
	  TIM_TimeBaseStructure.TIM_Prescaler = 960-1;
	  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	  /* (1) Select the active input TI1 (CC1S = 01),
	  program the input filter for 8 clock cycles (IC1F = 0011),
	  select the rising edge on CC1 (CC1P = 0, reset value)
	  and prescaler at each valid transition (IC1PS = 00, reset value) */
	  /* (2) Enable capture by setting CC1E */
	  /* (3) Enable interrupt on Capture/Compare */
	  /* (4) TS the filtering input off channel 1, and put into reset mode*/
	  /* (5) Enable counter */
	  TIM1->CCMR1 |= TIM_CCMR1_CC2S_1 | TIM_CCMR1_CC1S_0 | TIM_CCMR1_IC1F_0 | TIM_CCMR1_IC1F_1; /* (1) */
	  TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC2P; /* (2) */
	  TIM1->DIER |= TIM_DIER_CC1IE; /* (3) */
	  //TS 101: Filtered Timer Input 1 (TI1FP1)
	  TIM1->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_0 | TIM_SMCR_SMS_2;
	  TIM1->CR1 |= TIM_CR1_CEN; /* (5) */

	  /* (1) Select channel four to have both of the capture and compare events
	   * CC3S = 10 and CC4S = 01
	   * Capture at the clock frequency and filter with 8 sequential levels
	   */
	  TIM1->CCMR2 |= TIM_CCMR2_CC3S_1 | TIM_CCMR2_CC4S_0 | TIM_CCMR2_IC4F_0 | TIM_CCMR2_IC4F_1; // (1)
	  /*(2) Select channel 3 to trigger on rising edge and channel 4 to be falling edge
	   * This is used after the reset to determine the frequency and duty cycle
	   */
	  TIM1->CCER |= TIM_CCER_CC3E | TIM_CCER_CC4E | TIM_CCER_CC4P; // (2)
}

extern void pwm_in_init(){
	timer1_IT_config();
	timer2_IT_config();
	timer15_IT_config();
}

extern void init_interrupt_based_pwm_in(){
	left_motor_last_pulse = 0;
	top_right_motor_last_pulse = 0;
	top_left_motor_last_pulse = 0;
	middle_motor_last_pulse = 0;

	left_motor_pulse_before_tim_interupt = false;
	top_right_motor_pulse_before_tim_interupt = false;
	top_left_motor_pulse_before_tim_interupt = false;
	middle_motor_pulse_before_tim_interupt = false;
}

extern void config_exti_A0_A1(){//eleven
	  /* Enable the peripheral clock of GPIOA */
	  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	  /* Select mode */
	  /* Select input mode (00) on PA0 */
	  GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER1 | GPIO_MODER_MODER0));
	  //Select Pull down for A0 and A1
	  GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR0_1 | GPIO_PUPDR_PUPDR1_1);
	  /* Configure Syscfg, exti and nvic for pushbutton PA0 */
	  /* (1) PA0 as source input */
	  /* (2) unmask port 0 */
	  /* (3) Rising edge */
	  /* (4) Set priority */
	  /* (5) Enable EXTI0_1_IRQn */
	  SYSCFG->EXTICR[0] = (SYSCFG->EXTICR[0] & ~(SYSCFG_EXTICR1_EXTI1 | SYSCFG_EXTICR1_EXTI0)) | SYSCFG_EXTICR1_EXTI1_PA | SYSCFG_EXTICR1_EXTI0_PA; /* (1) */
	  EXTI->IMR |= (EXTI_IMR_MR1 | EXTI_IMR_MR0); /* (2) */
	  EXTI->RTSR |= (EXTI_RTSR_TR1 | EXTI_RTSR_TR0); /* (3) */
	  NVIC_SetPriority(EXTI0_1_IRQn, 0); /* (4) */
	  NVIC_EnableIRQ(EXTI0_1_IRQn); /* (5) */
}
