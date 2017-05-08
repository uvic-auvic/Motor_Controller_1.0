/**
 * STM32F0 internal temperature sensor readout utility
 * Tested on custom STM32F030F4 board.
 *
 * The STM32F0 ADC is used to continously convert
 * temperature sensor data and VREFINT data and DMA-copy
 * them to a static memory array.
 * In order to avoid spending significant amounts of time in
 * interrupt handlers, this implementation does not compute
 * the temperature in interrupt handlers. Instead,
 * the user must call readTemperatureData() in order to evaluate
 * the samples currently stored in the ADC and convert them to the temperature
 * value. Due to this strategy only the samples being present in the
 * ADC DMA buffer are averaged (yielding lower averaging sample count for
 * infrequent calls to readTemperatureData()) but the code does not require
 * CPU cycles (beyond those used by ChibiOS internals) inbetween calls to
 * readTemperatureData().
 *
 * In contrast to the ST example in RM0091, this code computes the temperature
 * in millicelsius using integer operations only.
 *
 * This implementation measures the VDDA analog supply voltage
 * quasi-synchronously to the temperature. Therefore,
 * no fixed or absolutely stable VDDA is required for proper operation
 * (tested with 3.0V and 3.3V). However, significant noise on the VDDA
 * line might also induce noise on the temperature output if
 * the sample buffer size is not large enough.
 *
 * Additionally, readCurrentVDDA() provides an utility method
 * to read the current VDDA voltage as millivolts.
 *
 * By using readTemperatureVDDA(), the user is able to compute both
 * the temperature and VDDA with only a single iteration over the sample
 * array. The actual VDDA is required for temperature normalization,
 * so returning it does not incur significant overhead.
 *
 * Designed for use with the ChibiOS HAL driver:
 * http://chibios.org
 *
 * Revision 1.0
 *
 * Copyright (c) 2015 Uli Koehler
 * http://techoverflow.net
 * Released under Apache License v2.0
 */
#ifndef __STM32F0_TEMPERATURE_H
#define __STM32F0_TEMPERATURE_H
#include <stdint.h>
/*
 * Register addresses were taken from DM00088500 (STM32F030 datasheet)
 * For non-STM32F030 microcontrollers register addresses
 * might need to be modified according to the respective datasheet.
 */
//Temperature sensor raw value at 30 degrees C, VDDA=3.3V
#define TEMP30_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7B8))
//Temperature sensor raw value at 110 degrees C, VDDA=3.3V
#define TEMP110_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7C2))
//Internal voltage reference raw value at 30 degrees C, VDDA=3.3V
#define VREFINT_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7BA))
/**
 * Configuration:
 * Defines the size of the ADC sample array.
 * Defining a larger value here will significantly increase
 * the amount of static RAM usage, but more values
 * will be used for averaging, leading to lower noise.
 */
#define ADC_TMPGRP_BUF_DEPTH 96
/**
 * Initialize the ADC and start continous sampling and DMA copying
 * of both temperature data and vrefint data.
 */
void initializeTemperatureADC(void);
/**
 * Compute the current temperature by evaluating the currently stored ADC samples.
 * @return The millicelsius absolute temperature, e.g. 12345 for 12.345 degrees C
 */
int32_t readTemperatureData(void);
/**
 * Compute the current VDDA by evaluating the stored ADC samples.
 * @return The absolute VDDA in mV, e.g. 3234 for 3.234V
 */
int16_t readCurrentVDDA(void);
/**
 * Data type that stores both a temperature and
 */
typedef struct {
    //Temperature in millidegrees C
    int32_t temperature;
    //Analog supply voltage in mV
    int32_t vdda;
} TemperatureVDDAResult;
/**
 * Compute both the temperature and the VDDA at once by
 * evaluating the stored ADC sample array
 */
TemperatureVDDAResult readTemperatureVDDA(void);
#endif //__STM32F0_TEMPERATURE_H
