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
#include "UART.h"
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
		GPIOA->ODR ^= GPIO_ODR_3;
		/* maintain LED C9 status for 200ms */
		vTaskDelay(500);
	}
}

void FSM(void *dummy){
	//initialize the FSM and UART
	FSM_Init();
	UART_init();

	inputBuffer.size = 0;

	//temporary storage to return from buffer
	char commandString[MAX_BUFFER_SIZE] = "";
	int tempVar;

	while(1){
		//it's important that this is while, if the task is accidentally awaken it
		//can't execute without having at least one item the input puffer
		while(inputBuffer.size == 0){
			//sleeps the task into it is notified to continue
			ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
		}
		//Write a statement here to do a string comparison on commands
		Buffer_pop(&inputBuffer, commandString);
		char arguement = commandString[3];
		commandString[3] = '\0';

		char tempOutputString[MAX_BUFFER_SIZE] = "";

		if(strcmp(commandString, "M1F") == 0){
			Motor_Speed(motor1, ((unsigned int)(arguement)), Forward);
		}
		else if(strcmp(commandString, "M1R") == 0){
			Motor_Speed(motor1, ((unsigned int)(arguement)), Reverse);
		}
		//Commented out because it's not tested
		/*else if(strcmp(commandString, "PW1") == 0){
			Motor_PWM(motor1, ((unsigned int)(arguement)* (10000 / 255)));
		}*/
		else if(strcmp(commandString, "RV1") == 0){
			tempVar = read_frequency(motor1) / CYCLES_PER_REV;
			tempOutputString[0] = (char)(tempVar & 0xFF);
			tempOutputString[1] = (char)((tempVar >> 8) & 0xFF);
			tempOutputString[2] = '\r';
			tempOutputString[3] = '\n';
			tempOutputString[4] = '\0';
			UART_push_out(tempOutputString);
		}else if(strcmp(commandString, "RVA") == 0){
			tempVar = read_frequency(motor1) / CYCLES_PER_REV;
			tempOutputString[0] = '(';
			tempOutputString[1] = (char)(tempVar & 0xFF);
			tempOutputString[2] = (char)((tempVar >> 8) & 0xFF);
			tempOutputString[3] = '\0';
			UART_push_out(tempOutputString);

			tempOutputString[0] = ')';
			tempOutputString[1] = '(';

			tempVar = read_frequency(motor2) / CYCLES_PER_REV;
			tempOutputString[2] = (char)(tempVar & 0xFF);
			tempOutputString[3] = (char)((tempVar >> 8) & 0xFF);
			tempOutputString[4] = '\0';

			UART_push_out(tempOutputString);

			tempOutputString[0] = ')';
			tempOutputString[1] = '(';

			tempVar = read_frequency(motor3) / CYCLES_PER_REV;

			tempOutputString[2] = (char)(tempVar & 0xFF);
			tempOutputString[3] = (char)((tempVar >> 8) & 0xFF);
			tempOutputString[4] = '\0';

			UART_push_out(tempOutputString);

			tempOutputString[0] = ')';
			tempOutputString[1] = '\r';
			tempOutputString[2] = '\n';
			tempOutputString[3] = '\0';

			UART_push_out(tempOutputString);

		}
		else if(strcmp(commandString, "SM1") == 0){
			Motor_Speed(motor1, 0, Forward);
		}
		else if(strcmp(commandString, "STP") == 0){
			Motors_Stop();
		}else if(strcmp(commandString, "RID") == 0){
			UART_push_out("Motor ");
			UART_push_out("Control");
			UART_push_out("ler\r\n");
		}
		else{
			UART_push_out("error: ");
			UART_push_out(commandString);
			UART_push_out("\r\n");
		}
	}
}

void measureTask(void* dummy){
	int dc1, dc2, dc15, f1, f2, f15;
	while(1){
		// Delay 1 seconds
		vTaskDelay(1000);

		// Read the duty cycle
		dc15 = read_duty_cycle_percent_10000(motor1);
		dc2 = read_duty_cycle_percent_10000(motor2);
		dc1 = read_duty_cycle_percent_10000(motor3);

		// Read the period
		f15 = read_frequency_hz(motor1);
		f2 = read_frequency_hz(motor2);
		f1 = read_frequency_hz(motor3);

		// Put a breakpoint there
		vTaskDelay(1);
	}
}

void vGeneralTaskInit(void){
    xTaskCreate(blinkyTask,
		(const signed char *)"blinkyTask",
		configMINIMAL_STACK_SIZE,
		NULL,                 // pvParameters
		tskIDLE_PRIORITY + 1, // uxPriority
		NULL              ); // pvCreatedTask */
    xTaskCreate(FSM,
		(const signed char *)"FSM",
		configMINIMAL_STACK_SIZE,
		NULL,                 // pvParameters
		tskIDLE_PRIORITY + 1, // uxPriority
		NULL              ); // pvCreatedTask */

    xTaskCreate(measureTask,
    	(const signed char*)"measureTask",
		configMINIMAL_STACK_SIZE,
		NULL,
		tskIDLE_PRIORITY + 1,
		NULL			  );
}

int
main(int argc, char* argv[])
{

	blink_led_init();
	pwm_in_init();

	//timer6_gen_system_clock_init();

	//Configure_GPIO_LED();
	//Configure_GPIO_USART1();
	//Configure_USART1();

	Motors_init();
	//timer_start();
	//config_exti_A0_A1();

	vGeneralTaskInit();

	/* Start the kernel.  From here on, only tasks and interrupts will run. */
	vTaskStartScheduler();

	// Should never get here
	while (1);
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
