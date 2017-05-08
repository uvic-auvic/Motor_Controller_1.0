#include "STM32F0Temperature.h"
#include <ch.h>
#include <hal.h>
#define ADC_TEMPGRP_NUM_CHANNELS   2 /* Temperature, VRefint */
/**
 * ADC sample buffer
 */
static adcsample_t temperatureVRefSamples[ADC_TEMPGRP_NUM_CHANNELS * ADC_TMPGRP_BUF_DEPTH];
/**
 * Continous 12-bit conversion of both CH16 (temperature)
 * and CH17 (VREFINT) without callbacks.
 * Uses the slowest possible sample rate in order to reduce
 * noise as much as possible.
 */
static const ADCConversionGroup adcTemperatureGroup = {
  TRUE,
  ADC_TEMPGRP_NUM_CHANNELS,
  NULL,
  NULL,
  ADC_CFGR1_CONT | ADC_CFGR1_RES_12BIT,             /* CFGR1 */
  ADC_TR(0, 0),                                     /* TR */
  ADC_SMPR_SMP_239P5,                               /* SMPR */
  ADC_CHSELR_CHSEL16 | ADC_CHSELR_CHSEL17           /* CHSELR */
};
void initializeTemperatureADC(void) {
    adcStart(&ADCD1, NULL);
    ADC->CCR |= ADC_CCR_TSEN | ADC_CCR_VREFEN;
    adcStartConversion(&ADCD1, &adcTemperatureGroup, temperatureVRefSamples, ADC_TMPGRP_BUF_DEPTH);
}
TemperatureVDDAResult readTemperatureVDDA(void) {
    //NOTE: Computation is performed in 32 bits, but result is converted to 16 bits later.s
    /**
     * Compute average of temperature sensor raw output
     * and vrefint raw output
     */
    int32_t tempAvg = 0;
    int32_t vrefintAvg = 0;
    //Samples are alternating: temp, vrefint, temp, vrefint, ...
    for(int i = 0; i < (ADC_TMPGRP_BUF_DEPTH * ADC_TEMPGRP_NUM_CHANNELS); i += 2) {
        tempAvg += temperatureVRefSamples[i];
        vrefintAvg += temperatureVRefSamples[i + 1];
    }
    tempAvg /= ADC_TMPGRP_BUF_DEPTH;
    vrefintAvg /= ADC_TMPGRP_BUF_DEPTH;
    /**
     * Compute temperature in millicelius.
     * Note that we need to normalize the value first by applying
     * the (actual VDDA / VDDARef) ratio.
     *
     * Note: VDDA_Actual = 3.3V * VREFINT_CAL / vrefintAvg
     * Therefore, the ratio mentioned above is equal to
     * q = VREFINT_CAL / vrefintAvg
     */
    //See RM0091 section 13.9
    int32_t temperature = ((tempAvg * (*VREFINT_CAL_ADDR)) / vrefintAvg) - (int32_t) *TEMP30_CAL_ADDR;
    temperature *= (int32_t)(110000 - 30000);
    temperature = temperature / (int32_t)(*TEMP110_CAL_ADDR - *TEMP30_CAL_ADDR);
    temperature += 30000;
    TemperatureVDDAResult ret = {
        temperature,
        (int16_t)((3300 * (*VREFINT_CAL_ADDR)) / vrefintAvg)
    };
    return ret;
}
int32_t readTemperatureData(void) {
    return readTemperatureVDDA().temperature;
}
int16_t readCurrentVDDA(void) {
    return readTemperatureVDDA().vdda;
}
