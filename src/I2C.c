/*
 * I2C.c
 *
 *  Created on: May 15, 2017
 *      Author: auvic
 */
#include "I2C.h"
#include "stm32f0xx.h"

#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

TaskHandle_t xTaskToNotify = NULL;
static uint8_t * data_pipeline;

static void I2C2_set_address_and_byte_count(uint8_t address, uint8_t bytes_to_send){
	I2C1->CR2 =  (address<<1);
	I2C1->CR2 |= (bytes_to_send << 16);
}

static void reset_possibly_set_config_options(){
	I2C1->CR1 &= (~(I2C_CR1_RXIE | I2C_CR1_TXIE | I2C_CR1_TCIE));
	I2C1->CR2 &= (~I2C_CR2_RD_WRN);
}

static void set_task_to_notify_handle(){
	/* Store the handle of the calling task. */
	xTaskToNotify = xTaskGetCurrentTaskHandle();
}

void i2c_Init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_1);

	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	I2C_InitTypeDef  I2C_InitStructure;

  /* I2C configuration */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  I2C1->CR1 = I2C_CR1_PE;

  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;//
  I2C_InitStructure.I2C_DigitalFilter = 0x00;//
  I2C_InitStructure.I2C_OwnAddress1 = 0x00;//
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;//
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;//
  I2C_InitStructure.I2C_Timing =0x00B01A4B; //0x00201D2B

  //  /* (1) Timing register value is computed with the AN4235 xls file,
  //   fast Mode @400kHz with I2CCLK = 48MHz, rise time = 140ns, fall time = 40ns */


  /* Apply I2C configuration after enabling it */
  I2C_Init(I2C1, &I2C_InitStructure);

  /* sEE_I2C Peripheral Enable */
  I2C_Cmd(I2C1, ENABLE);

  I2C1->CR2 =  I2C_CR2_AUTOEND;

  /* Configure IT */
  /* (4) Set priority for I2C2_IRQn */
  /* (5) Enable I2C2_IRQn */
  NVIC_SetPriority(I2C1_IRQn, 6); /* (4) */
  NVIC_EnableIRQ(I2C1_IRQn); /* (5) */
}

extern void I2C1_IRQHandler(void){
	if((I2C1->ISR & I2C_ISR_TXIS) == I2C_ISR_TXIS){
		//I2C1->TXDR = data_to_send_update();
	}
	else if((I2C1->ISR & I2C_ISR_RXNE) == I2C_ISR_RXNE){
		*data_pipeline = (I2C1->RXDR);
		data_pipeline++;
	}
	else if((I2C1->ISR & I2C_ISR_TC) == I2C_ISR_TC){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		/* At this point xTaskToNotify should not be NULL as a transmission was
		in progress. */
		configASSERT( xTaskToNotify != NULL );

		/* Notify the task that the transmission is complete. */
		vTaskNotifyGiveFromISR( xTaskToNotify, &xHigherPriorityTaskWoken );
		I2C1->CR2 |=  I2C_CR2_STOP;
	}
}

extern void I2C2_send_message_no_cb(uint8_t message, uint8_t address){
	I2C2_set_address_and_byte_count(address, 1);
	I2C1->TXDR = message;
	reset_possibly_set_config_options();
	set_task_to_notify_handle();
	I2C1->CR1 |= I2C_CR1_TCIE;
	I2C1->CR2 |= I2C_CR2_START;
}

extern void I2C2_recv_message_with_cb(uint8_t address, uint8_t bytes_to_send, uint8_t * recv_data){
	//Setting the address and the amount of bytes we want to send
	I2C2_set_address_and_byte_count(address, bytes_to_send);
	//reseting the I2C controller
	reset_possibly_set_config_options();
	//updating the static variables for the transaction
	data_pipeline = recv_data;
	set_task_to_notify_handle();

	//start transaction
	I2C1->CR1 |= I2C_CR1_RXIE | I2C_CR1_TCIE;
	I2C1->CR2 |= I2C_CR2_RD_WRN | I2C_CR2_START;
}
