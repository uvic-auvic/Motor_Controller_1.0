//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

#include "UART_Receiver.h"
#include <string.h>

//Definitions
#define MIN_COMMAND_LENGTH (4)
#define MAX_COMMAND_LENGTH (5)

#define MAX_OUPUT_DATA (10)

/* Private variables ---------------------------------------------------------*/
int transfer_in_progress;
uint8_t send = 0;
char stringtosend[MAX_OUPUT_DATA] = "\0";

char chars_recv[MAX_COMMAND_LENGTH] = ""; //Using for storing the previous portion of the command
int curr_data_recv_idx = 0; //Storing how far we are in to the current command

static void ResetCommBuffer(void){
	int i=0;
	for(; i<MAX_COMMAND_LENGTH; i++){
		chars_recv[i] = 0;
	}
	curr_data_recv_idx=0;
}

static void AppendToCommBuffer(char data){
	chars_recv[curr_data_recv_idx] = data;
	curr_data_recv_idx++;
}


// ----------------------------------------------------------------------------

/**
  * @brief  This function :
             - Enables GPIO clock
             - Configures the Green LED pin on GPIO PC9
             - Configures the Orange LED pin on GPIO PC8
  * @param  None
  * @retval None
  */
__INLINE void Configure_GPIO_LED(void)
{
  /* Enable the peripheral clock of GPIOC */
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

  /* Select output mode (01) on PC8 and PC9 */
  GPIOC->MODER = (GPIOC->MODER & ~(GPIO_MODER_MODER8)) \
                 | (GPIO_MODER_MODER8_0);
}

/**
  * @brief  This function :
             - Enables GPIO clock
             - Configures the USART1 pins on GPIO PB6 PB7
  * @param  None
  * @retval None
  */
__INLINE void Configure_GPIO_USART1(void)
{
  /* Enable the peripheral clock of GPIOA */
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

  /* GPIO configuration for USART1 signals */
  /* (1) Select AF mode (10) on PA9 and PA10 */
  /* (2) AF1 for USART1 signals */
  GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER9|GPIO_MODER_MODER10))\
                 | (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1); /* (1) */
  GPIOA->AFR[1] = (GPIOA->AFR[1] &~ (GPIO_AFRH_AFRH1 | GPIO_AFRH_AFRH2))\
                  | (1 << (1 * 4)) | (1 << (2 * 4)); /* (2) */
}

/**
  * @brief  This function configures USART1.
  * @param  None
  * @retval None
  */
__INLINE void Configure_USART1(void)
{
  /* Enable the peripheral clock USART1 */
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

  /* Configure USART1 */
  /* (1) oversampling by 16, 9600 baud */
  /* (2) 8 data bit, 1 start bit, 1 stop bit, no parity, reception mode */
  USART1->BRR = 480000 / 96; /* (1) */
  USART1->CR1 = USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_UE | USART_CR1_TE ; /* (2) */

  /* polling idle frame Transmission */
   while((USART1->ISR & USART_ISR_TC) != USART_ISR_TC)
   {
     /* add time out here for a robust application */
   }
   USART1->ICR |= USART_ICR_TCCF;/* clear TC flag */
   USART1->CR1 |= USART_CR1_TCIE;/* enable TC interrupt */

  /* Configure IT */
  /* (3) Set priority for USART1_IRQn */
  /* (4) Enable USART1_IRQn */
  NVIC_SetPriority(USART1_IRQn, 0); /* (3) */
  NVIC_EnableIRQ(USART1_IRQn); /* (4) */

  transfer_in_progress = 0;
}

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}


/**
  * @brief  This function handles EXTI 0 1 interrupt request.
  * @param  None
  * @retval None
  */

/**
  * @brief  This function handles USART1 interrupt request.
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
  uint8_t chartoreceive = 0;

  //Sending out the string
  if((USART1->ISR & USART_ISR_TC) == USART_ISR_TC)
    {
      if(send == sizeof(stringtosend))
      {
    	transfer_in_progress = 0;
        send=0;
        USART1->ICR |= USART_ICR_TCCF; /* Clear transfer complete flag */
      }
      else
      {
        /* clear transfer complete flag and fill TDR with a new char */
        USART1->TDR = stringtosend[send++];
      }
    }

  else if((USART1->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
  {
    chartoreceive = (uint8_t)(USART1->RDR); /* Receive data, clear flag */
    // \n cases
    if(chartoreceive=='\n' || chartoreceive == '\r'){
    	// We don't want to use the \r, less to think about later with perfect strings
    	AppendToCommBuffer('\0');
    	if(curr_data_recv_idx < MIN_COMMAND_LENGTH){
    		Send_to_Odroid("tiny\r\n");
    	}else{
    		// curr_data_recv_idx + 1 because of how arrays work
    		command_recv(chars_recv, curr_data_recv_idx + 1);
    	}
    	ResetCommBuffer();
    }
    //Too many characters without \n

    //No need to check \n in the statement as it will be checked by the above statement
    else if(curr_data_recv_idx == MAX_COMMAND_LENGTH){
    	Send_to_Odroid("long\r\n");
    	ResetCommBuffer();
    }else{
    	AppendToCommBuffer(chartoreceive);
    }
  }
  else
  {
          NVIC_DisableIRQ(USART1_IRQn); /* Disable USART1_IRQn */
  }
}

extern void UART_push_out(char* mesg){
	transfer_in_progress = 1;
	send = 0;
	strcpy(stringtosend, mesg);
	/* start USART transmission */
	USART1->TDR = stringtosend[send++]; /* Will initialize TC if TXE */
	return;
}

extern int check_UART_busy(){
	return transfer_in_progress;
}

/**
  * @}
  */

// ----------------------------------------------------------------------------
