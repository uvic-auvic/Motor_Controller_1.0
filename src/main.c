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

#include "FreeRTOSConfig.h"

#include "FreeRTOS.h"
#include "task.h"

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compilfer diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

void blinkyTask(void *dummy){
	while(1){
		GPIOC->ODR ^= GPIO_ODR_9;
		/* maintain LED C9 status for 200ms */
		vTaskDelay(500);
	}
}

void vGeneralTaskInit(void){
    xTaskCreate(blinkyTask,
		(const signed char *)"blinkyTask",
		configMINIMAL_STACK_SIZE,
		NULL,                 // pvParameters
		tskIDLE_PRIORITY + 1, // uxPriority
		NULL              ); // pvCreatedTask */
}

int
main(int argc, char* argv[])
{

	blink_led_init();

	pwm_in_init();

	//timer6_gen_system_clock_init();

	Configure_GPIO_LED();
	Configure_GPIO_USART1();
	Configure_USART1();

	Motors_init();
	//timer_start();
	config_exti_A0_A1();

	vGeneralTaskInit();

	/* Start the kernel.  From here on, only tasks and interrupts will run. */
	vTaskStartScheduler();

	// Should never get here
	while (1);
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
