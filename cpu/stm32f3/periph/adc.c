/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_stm32f3
 * @{
 *
 * @file
 * @brief       Low-level ADC driver implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdint.h>
#include <string.h>

#include "cpu.h"
#include "periph/adc.h"
#include "periph_conf.h"

/* guard in case that no ADC device is defined */
#if ADC_NUMOF


typedef struct {
    uint8_t resolution;
} adc_config_t;


adc_config_t config[ADC_NUMOF];


int adc_init(adc_t dev, adc_precision_t resolution)
{
    ADC_TypeDef *adc = 0;

    adc_poweron(dev);

    switch (dev) {
#if ADC_0_EN
        case ADC_0:
            adc = ADC_0_DEV;
            ADC_0_PORT_CLKEN();
            ADC_0_PORT->MODER |= (3 << ADC_0_CH0_PIN | 3 << ADC_0_CH1_PIN |
                                  3 << ADC_0_CH2_PIN);
            break;
#endif
#if ADC_1_EN
        case ADC_1:
            adc = ADC_1_DEV;
            ADC_1_PORT_CLKEN();
            ADC_1_PORT->MODER |= (3 << ADC_1_CH0_PIN | 3 << ADC_1_CH1_PIN |
                                  3 << ADC_1_CH2_PIN);
            break;
#endif
    }

    /* reset control registers */
    adc->CR = 0;
    adc->CFGR = 0;

    /* set resolution */
    config[dev].resolution = (6 + (2 * resolution));

    switch (resolution) {
        case ADC_RES_6BIT:
            adc->CFGR |= ADC_CFGR_RES_0 | ADC_CFGR_RES_1;
            break;
        case ADC_RES_8BIT:
            adc->CFGR |= ADC_CFGR_RES_1;
            break;
        case ADC_RES_10BIT:
            adc->CFGR |= ADC_CFGR_RES_0;
            break;
        case ADC_RES_12BIT:
            break;
        case ADC_RES_14BIT:
        case ADC_RES_16BIT:
            adc_poweroff(dev);
            return -1;
            break;
    }

    /* enable the ADC module */
    adc->CR |= ADC_CR_ADEN;

    return 0;
}

int adc_sample(adc_t dev, int channel)
{
    ADC_TypeDef *adc;

    switch (dev) {
#if ADC_0_EN
        case ADC_0:
            adc = ADC_0_DEV;
            switch (channel) {
                case 0:
                    adc->SQR1 = ((ADC_0_CH0 & 0x1f) << 6);
                    break;
                case 1:
                    adc->SQR1 = ((ADC_0_CH1 & 0x1f) << 6);
                    break;
                case 2:
                    adc->SQR1 = ((ADC_0_CH2 & 0x1f) << 6);
                    break;
                default:
                    return -1;
            }
            break;
#endif
#if ADC_1_EN
        case ADC_1:
            adc = ADC_1_DEV;
            switch (channel) {
                case 0:
                    adc->SQR1 = ((ADC_1_CH0 & 0x1f) << 6);
                    break;
                case 1:
                    adc->SQR1 = ((ADC_1_CH1 & 0x1f) << 6);
                    break;
                case 2:
                    adc->SQR1 = ((ADC_1_CH2 & 0x1f) << 6);
                    break;
                default:
                    return -1;
            }
            break;
#endif
        default:
            return -1;
    }

    /* start single conversion */
    adc->CR |= ADC_CR_ADSTART;
    /* wait until conversion is complete */
    while (!(adc->ISR & ADC_ISR_EOC));
    /* read and return result */
    return (int)adc->DR;
}

void adc_poweron(adc_t dev)
{
    switch (dev) {
#if ADC_0_EN
        case ADC_0:
            ADC_0_CLKEN();
            break;
#endif
#if ADC_1_EN
        case ADC_1:
            ADC_1_CLKEN();
            break;
#endif
    }
}

void adc_poweroff(adc_t dev)
{
    switch (dev) {
#if ADC_0_EN
        case ADC_0:
            ADC_0_CLKDIS();
            break;
#endif
#if ADC_1_EN
        case ADC_1:
            ADC_1_CLKDIS();
            break;
#endif
    }
}

/**
 * @brief Helper function to map a converted value to the given integer range.
 *
 * This function is useful for converting sampled ADC values into their physical representation.
 *
 * The min value is asserted to be smaller than the max value.
 *
 * @param[in] dev           the ADC device to read the precision value from (as input interval)
 * @param[in] value         the value to map
 * @param[in] min           the lower bound of the target interval
 * @param[in] max           the upper bound of the target interval
 *
 * @return                  the mapped value
 */
int adc_map(adc_t dev, int value, int min, int max)
{
    return 0;
}

/**
 * @brief Helper function to map a converted value to the given float range
 *
 * @see adc_map
 *
 * @param[in] dev           the ADC device to read the precision value from (as input interval)
 * @param[in] value         the value to map
 * @param[in] min           the lower bound of the target interval
 * @param[in] max           the upper bound of the target interval
 *
 * @return                  the mapped value
 */
float adc_mapf(adc_t dev, int value, float min, float max)
{
    return 0.0;
}

#endif /* ADC_NUMOF */
