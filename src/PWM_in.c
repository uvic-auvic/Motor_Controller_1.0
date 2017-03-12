#include "PWM_in.h"
#include "Timer.h"
#include "stm32f0xx.h"
#include "stm32f0xx_misc.h"
#include "motors.h"
#include <stdbool.h>

enum {TIM_CHANNEL_1 = 1, TIM_CHANNEL_2};

// Reads the duty cycle of the PWM signal for the specified motor (100% mapped to 10000)
extern int read_duty_cycle_percent_10000(motor m){
	int total_clock_cycles = 0;
	int high_clock_cycles = 0;
	switch(m){
	case motor1:
		total_clock_cycles = TIM15->CCR1;
		high_clock_cycles = TIM15->CCR2;
		break;
	case motor2:
		total_clock_cycles = TIM2->CCR2;
		high_clock_cycles = TIM2->CCR1;
		break;
	case motor3:
		total_clock_cycles = TIM1->CCR1;
		high_clock_cycles = TIM1->CCR2;
		break;
	}

	// Make sure I don't divide by 0
	if(total_clock_cycles)
		return ((high_clock_cycles * 10000) / total_clock_cycles); //The hundred is how percent work
	else return 0;
}

// Reads to the frequency of the PWM signal (Hz) of the specified motor
extern int read_frequency_hz(motor m){
	int period_clk_cycles = 0;
	switch(m){
	case motor1:
		period_clk_cycles = TIM15->CCR1;
		break;
	case motor2:
		period_clk_cycles = TIM2->CCR2;
		break;
	case motor3:
		period_clk_cycles = TIM1->CCR1;
		break;
	}

	// Make sure I don't divide by 0
	if(period_clk_cycles)
		return CLOCK_PERIOD / period_clk_cycles;
	else return 0;
}

/* Configures the specified timer to capture a PWM input from the
 * specified timer channel. The duty cycle and period of the PWM signal
 * is stored in CCR1 and CCR2, depending on what channel the timer uses.
 *
 * Note: Only channels 1 & 2 will work, as channels 3 & 4 don't have filtered
 * input to the slave control (can't reset timer on rising edge) so they don't
 * work with PWM input.
 */
static void TIM_ConfigPWMIn(TIM_TypeDef* TIMx, uint8_t channel){
	if(channel == TIM_CHANNEL_1){
		// Note: CC - Capture/Compare
		// Set CC1 and CC2 to channel 1, and make an 8 clock cycle filter on channel 1
		TIMx->CCMR1 |= TIM_CCMR1_CC2S_1 | TIM_CCMR1_CC1S_0 | TIM_CCMR1_IC1F_0 | TIM_CCMR1_IC1F_1;

		// Enable CCR1 update for rising edge, CCR2 update for falling edge
		TIMx->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC2P;

		// Make slave mode reset on rising edge from channel 1
		TIMx->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_0 | TIM_SMCR_SMS_2;

		// Enable CC and only fire interrupt on overflow/underflow
		TIMx->CR1 |= TIM_CR1_CEN | TIM_CR1_URS;
	}
	else if(channel == TIM_CHANNEL_2){
		// Note: CC - Capture/Compare
		// Set CC1 and CC2 to channel 2, and make an 8 clock cycle filter on channel 2
		TIMx->CCMR1 |= TIM_CCMR1_CC1S_1 | TIM_CCMR1_CC2S_0 | TIM_CCMR1_IC2F_0 | TIM_CCMR1_IC2F_1;

		// Enable CCR2 update for rising edge, CCR1 update for falling edge
		TIMx->CCER |= TIM_CCER_CC2E | TIM_CCER_CC1E | TIM_CCER_CC1P;

		// Make slave mode reset on rising edge from channel 2
		TIMx->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_1 | TIM_SMCR_SMS_2;

		// Enable CC and only fire interrupt on overflow/underflow
		TIMx->CR1 |= TIM_CR1_CEN | TIM_CR1_URS;
	}
}

/* Clears the CCR1 and CCR2 registers for the specified timer. This should be
 * used for timers configured in PWM input mode. To write to these registers, the
 * registers must be configured to be in output mode, otherwise they are read-only.
 * These registers are configured differently for different timer channels.
 */
static void TIM_ResetCCR(TIM_TypeDef* TIMx, uint8_t channel)
{
	if(channel == TIM_CHANNEL_1)
	{
		// Disable CC
		TIMx->CCER ^= TIM_CCER_CC1E | TIM_CCER_CC2E;

		// Configure timer CCR as output so they are writable
		TIMx->CCMR1 ^= TIM_CCMR1_CC2S_1 | TIM_CCMR1_CC1S_0;

		// Reset CCR registers
		TIMx->CCR1 = 0;
		TIMx->CCR2 = 0;

		// Revert timer CCR registers back to original
		TIMx->CCMR1 |= TIM_CCMR1_CC2S_1 | TIM_CCMR1_CC1S_0;

		// Re-enable CC
		TIMx->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
	}
	else if(channel == TIM_CHANNEL_2)
	{
		// Disable CC
		TIMx->CCER ^= TIM_CCER_CC1E | TIM_CCER_CC2E;

		// Configure timer CCR as output so they are writable
		TIMx->CCMR1 ^= TIM_CCMR1_CC1S_1 | TIM_CCMR1_CC2S_0;

		// Reset CCR registers
		TIMx->CCR1 = 0;
		TIMx->CCR2 = 0;

		// Revert timer CCR registers back to original
		TIMx->CCMR1 |= TIM_CCMR1_CC1S_1 | TIM_CCMR1_CC2S_0;

		// Re-enable CC
		TIMx->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
	}
}

// Configures timer1 to work in PWM input mode
static void timer1_IT_config(){
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

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

	/* Enable the TIM1 interrupt from the interrupt table */
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_UP_TRG_COM_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable Update interrupt for TIM1 */
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

	/* Setup counting structure for TIM1 */
	TIM_TimeBaseStructure.TIM_Period = 50000-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 960-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/* Configure TIM1 Channel 1 to PWM Input mode */
	TIM_ConfigPWMIn(TIM1, TIM_CHANNEL_1);
}

// Configures timer2 to work in PWM input mode
static void timer2_IT_config(){
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* GPIOA clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* GPIOA Configuration: TIM2 CH2 (PA1) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // Input/Output controlled by peripheral
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // Button to ground expectation
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Connect TIM2 pins to AF */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_2);

	/* Enable the TIM2 interrupt from the interrupt table */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable Update interrupt for TIM2 */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	/* Setup counting structure for TIM2 */
	TIM_TimeBaseStructure.TIM_Period = 50000-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 960-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* Configure TIM2 Channel 2 to PWM Input mode */
	TIM_ConfigPWMIn(TIM2, TIM_CHANNEL_2);
}

// Configure 15 to work in PWM input mode
static void timer15_IT_config(){
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

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
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Connect TIM15 pins to AF */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_1);

	/* Enable the TIM15 interrupt from the interrupt table */
	NVIC_InitStructure.NVIC_IRQChannel = TIM15_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable Update interrupt for TIM15 */
	TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);

	/* Setup counting structure for TIM15 */
	TIM_TimeBaseStructure.TIM_Period = 50000-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 960-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM15, &TIM_TimeBaseStructure);

	/* Configure TIM15 Channel 1 to PWM Input mode */
	TIM_ConfigPWMIn(TIM15, TIM_CHANNEL_1);
}

// Initializes all timers configured in PWM input mode
extern void pwm_in_init(){
	timer1_IT_config();
	timer2_IT_config();
	timer15_IT_config();
}

// Resets CCR1 and CCR2 when a timer overflow event occurs
void TIM1_BRK_UP_TRG_COM_IRQHandler(void){
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET){
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		TIM_ResetCCR(TIM1, TIM_CHANNEL_1);
	}
}

// Resets CCR1 and CCR2 when a timer overflow event occurs
void TIM2_IRQHandler(void){
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET){
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		TIM_ResetCCR(TIM2, TIM_CHANNEL_2);
	}
}

// Resets CCR1 and CCR2 when a timer overflow event occurs
void TIM15_IRQHandler(void){
	if (TIM_GetITStatus(TIM15, TIM_IT_Update) != RESET){
		TIM_ClearITPendingBit(TIM15, TIM_IT_Update);
		TIM_ResetCCR(TIM15, TIM_CHANNEL_1);
	}
}
