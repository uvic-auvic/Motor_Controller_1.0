//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
//#include "diag/Trace.h"

#include "Timer.h"
#include "BlinkLed.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_tim.h"
#include <stm32f0xx_rcc.h>
#include "UART_Receiver.h"
#include "FSM.h"
#include "Buffer.h"
#include "motors.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_syscfg.h"
#include "PWM_in.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F0 led blink sample (trace via $(trace)).
//
// In debug configurations, demonstrate how to print a greeting message
// on the trace device. In release configurations the message is
// simply discarded.
//
// Then demonstrates how to blink a led with 1 Hz, using a
// continuous loop and SysTick delays.
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the $(trace) output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//
// The external clock frequency is specified as a preprocessor definition
// passed to the compiler via a command line option (see the 'C/C++ General' ->
// 'Paths and Symbols' -> the 'Symbols' tab, if you want to change it).
// The value selected during project creation was HSE_VALUE=8000000.
//
// Note: The default clock settings take the user defined HSE_VALUE and try
// to reach the maximum possible system clock. For the default 8 MHz input
// the result is guaranteed, but for other values it might not be possible,
// so please adjust the PLL settings in system/src/cmsis/system_stm32f0xx.c
//

//Define onboard LED’s
#define GreenLED GPIO_Pin_8
#define BlueLED GPIO_Pin_9
#define LEDGPIO GPIOC
#define LEDToggleValue ((uint16_t) 3000)

// ----- Timing definitions -------------------------------------------------

// Keep the LED on for 2/3 of a second.
#define BLINK_ON_TICKS  (TIMER_FREQUENCY_HZ * 3 / 4)
#define BLINK_OFF_TICKS (TIMER_FREQUENCY_HZ - BLINK_ON_TICKS)

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compilfer diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

int
main(int argc, char* argv[])
{

  init_FSM();

  blink_led_init();

  timer1_IT_config();

  timer6_gen_system_clock_init();

  Configure_GPIO_LED();
  Configure_GPIO_USART1();
  Configure_USART1();

  Motors_init();
  //timer_start();
  config_exti_A0_A1();

  // Infinite loop
  while (1)
    {
	  FSM_do();
	  continue;
    }
  // Infinite loop, never return.
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		//GPIO_WriteBit(GPIOC, GPIO_Pin_8, !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8));
	}
}

void TIM6_DAC_IRQHandler(void){
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
		GPIOC->ODR ^= GPIO_ODR_9;
	}
}

void EXTI0_1_IRQHandler(void)
{
	if((EXTI->PR & EXTI_PR_PR0) == EXTI_PR_PR0){
		motor_pulse(left_side);
		GPIOC->ODR |= GPIO_ODR_8; /* turn on blue LED */
		EXTI->PR |= EXTI_PR_PR0;
	}
	if((EXTI->PR & EXTI_PR_PR1) == EXTI_PR_PR1){
		GPIOC->ODR &= ~(GPIO_ODR_8); /* turn off blue LED */
		EXTI->PR |= EXTI_PR_PR1;
	}
}

void TIM16_IRQHandler(void)//Once per second
{
	if (TIM_GetITStatus(TIM16, TIM_IT_Update) != RESET){
		TIM_ClearITPendingBit(TIM16, TIM_IT_Update);
	}
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
