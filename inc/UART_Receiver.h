//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

#ifndef UART_Receiver_H_
#define UART_Receiver_H_

#include "stm32f0xx.h"
#include "FSM.h"

/* Public function prototypes -----------------------------------------------*/
extern void Configure_GPIO_LED(void);
extern void Configure_GPIO_USART1(void);
extern void Configure_USART1(void);
extern void UART_push_out(char* mesg);
extern int check_UART_busy(void);
// ----------------------------------------------------------------------------

/* Private function prototypes -----------------------------------------------
static void ResetCommBuffer(void);
static void AppendToCommBuffer(char data);
// --------------------------------------------------------------------------*/

#endif // UART_Receiver_H_
