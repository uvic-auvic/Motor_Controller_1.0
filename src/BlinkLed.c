//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

#include "BlinkLed.h"

// ----------------------------------------------------------------------------

void
blink_led_init()
{
  // Enable GPIO Peripheral clock
  //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;

  //Andy's attempt at blinking an LED

  //Enable the A port to be used
  RCC->AHBENR |= RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOA;

  //For now use the GPIO_Init function, to save time
  //Be careful because not erasing any of the past settings so failure to overwrite will keep them

  //Select port 3 to be our toggle port
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_RESET);
}

/*(void
pulses_in_config()
{
	GPIOC clock enable
	RCC_AHB1PeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
} */

void
pwm_led_init()
{
	//Configure the output on the pins
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Clock for GPIOD */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	/*Alternating functions for pins*/
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_2);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
}

// ----------------------------------------------------------------------------
