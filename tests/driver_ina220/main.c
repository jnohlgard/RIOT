/*
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Test application for the INA220 sensor driver
 *
 * @author      Joakim Gebart <joakim.gebart@eistec.se>
 *
 * @}
 */

#ifndef TEST_INA220_I2C
#error "TEST_INA220_I2C not defined"
#endif
#ifndef TEST_INA220_ADDR
#error "TEST_INA220_ADDR not defined"
#endif

#include <stdio.h>

#include "vtimer.h"
#include "ina220.h"

/* Use the following configuration:
 *
 *  - Continuous measurements, both shunt and bus voltage
 *  - +/- 320 mV Vshunt range
 *  - 32 V maximum bus voltage
 *  - 12 bit ADC resolution, no hardware averaging
 */
#define CONFIG   (INA220_MODE_CONTINUOUS_SHUNT_BUS | INA220_RANGE_320MV | \
                  INA220_BRNG_32V_FSR | INA220_SADC_12BIT | INA220_BADC_12BIT)
#define CALIBRATION (4096)
#define SLEEP    (100 * 1000U)

int main(void)
{
    ina220_t dev;
    int16_t val;

    puts("INA220 sensor driver test application\n");
    printf("Initializing I2C_%i... ", TEST_INA220_I2C);
    if (i2c_init_master(TEST_INA220_I2C, I2C_SPEED_FAST) < 0) {
        return -1;
    }

    printf("Initializing INA220 sensor at I2C_%i, address 0x%02x... ",
        TEST_INA220_I2C, TEST_INA220_ADDR);
    if (ina220_init(&dev, TEST_INA220_I2C, TEST_INA220_ADDR) == 0) {
        puts("[OK]\n");
    } else {
        puts("[Failed]");
        return 1;
    }
    puts("Set configuration register");
    if (ina220_set_config(&dev, CONFIG) == 0) {
        puts("[OK]\n");
    } else {
        puts("[Failed]");
        return 1;
    }

    puts("Set calibration register");
    if (ina220_set_calibration(&dev, CALIBRATION) == 0) {
        puts("[OK]\n");
    } else {
        puts("[Failed]");
        return 1;
    }

    while (1) {
        ina220_read_shunt(&dev, &val);

        printf("shunt: %6d", val);

        ina220_read_bus(&dev, &val);

        printf("\tbus: %6d", val);

        ina220_read_current(&dev, &val);

        printf("\tcurrent: %6d", val);

        ina220_read_power(&dev, &val);

        printf("\tpower: %6d\n", val);

        vtimer_usleep(SLEEP);
    }

    return 0;
}
