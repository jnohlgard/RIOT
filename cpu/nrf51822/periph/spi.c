/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_nrf51822
 * @{
 *
 * @file        gpio.c
 * @brief       Low-level SPI driver implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "cpu.h"
#include "periph/spi.h"
#include "periph_conf.h"

/* guard this file in case no SPI device is defined */
#if SPI_NUMOF

/* static port mapping */
static NRF_SPI_Type *const spi[] = {
#if SPI_0_EN
    SPI_0_DEV,
#endif
#if SPI_1_EN
    SPI_1_DEV
#endif
};

int spi_init_master(spi_t dev, spi_conf_t conf, spi_speed_t speed)
{
    if (dev >= SPI_NUMOF) {
        return -1;
    }

    spi_poweron(dev);

    switch(dev) {
#if SPI_0_EN
        case SPI_0:
            /* configure direction of used pins */
            NRF_GPIO->DIRSET = (1 << SPI_0_PIN_MOSI) | (1 << SPI_0_PIN_SCK);
            NRF_GPIO->DIRCLR = (1 << SPI_0_PIN_MISO);
            /* select pins to be used by SPI */
            spi[dev]->PSELMOSI = SPI_0_PIN_MOSI;
            spi[dev]->PSELMISO = SPI_0_PIN_MISO;
            spi[dev]->PSELSCK = SPI_0_PIN_SCK;
            break;
#endif
#if SPI_1_EN
        case SPI_1:
            /* configure direction of used pins */
            NRF_GPIO->DIRSET = (1 << SPI_1_PIN_MOSI) | (1 << SPI_1_PIN_SCK);
            NRF_GPIO->DIRCLR = (1 << SPI_1_PIN_MISO);
            /* select pins to be used by SPI */
            spi[dev]->PSELMOSI = SPI_1_PIN_MOSI;
            spi[dev]->PSELMISO = SPI_1_PIN_MISO;
            spi[dev]->PSELSCK = SPI_1_PIN_SCK;
            break;
#endif
        default:
            return -1;
    }

    /* configure SPI mode */
    switch (conf) {
        case SPI_CONF_FIRST_RISING:
            spi[dev]->CONFIG = (SPI_CONFIG_CPOL_ActiveHigh << 2) | (SPI_CONFIG_CPHA_Leading << 1);
            break;
        case SPI_CONF_SECOND_RISING:
            spi[dev]->CONFIG = (SPI_CONFIG_CPOL_ActiveHigh << 2) | (SPI_CONFIG_CPHA_Trailing << 1);
            break;
        case SPI_CONF_FIRST_FALLING:
            spi[dev]->CONFIG = (SPI_CONFIG_CPOL_ActiveLow << 2) | (SPI_CONFIG_CPHA_Leading << 1);
            break;
        case SPI_CONF_SECOND_FALLING:
            spi[dev]->CONFIG = (SPI_CONFIG_CPOL_ActiveLow << 2) | (SPI_CONFIG_CPHA_Trailing << 1);
            break;
    }

    /* select bus speed */
    switch (speed) {
        case SPI_SPEED_100KHZ:          /* 125 KHz for this device */
            spi[dev]->FREQUENCY = SPI_FREQUENCY_FREQUENCY_K125;
            break;
        case SPI_SPEED_400KHZ:          /* 500 KHz for this device */
            spi[dev]->FREQUENCY = SPI_FREQUENCY_FREQUENCY_K500;
            break;
        case SPI_SPEED_1MHZ:            /* 1 MHz for this device */
            spi[dev]->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M1;
            break;
        case SPI_SPEED_5MHZ:            /* 4 MHz for this device */
            spi[dev]->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M4;
            break;
        case SPI_SPEED_10MHZ:           /* 8 MHz for this device */
            spi[dev]->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M8;
            break;
    }

    /* finally enable the device */
    spi[dev]->ENABLE = 1;
    return 0;
}

int spi_init_slave(spi_t dev, spi_conf_t conf, char (*cb)(char data))
{
    return -1;      /* the NRF51822 does not support SPI slave mode */
}

int spi_transfer_byte(spi_t dev, char out, char *in)
{
    return spi_transfer_bytes(dev, &out, in, 1);
}

int spi_transfer_bytes(spi_t dev, char *out, char *in, unsigned int length)
{
    if (dev >= SPI_NUMOF) {
        return -1;
    }

    for (int i = 0; i < length; i++) {
        spi[dev]->EVENTS_READY = 0;
        spi[dev]->TXD = (uint8_t)out[i];
        while (spi[dev]->EVENTS_READY != 1);
        if (in) {
            in[i] = (char)spi[dev]->RXD;
        }
    }

    return length;
}

int spi_transfer_reg(spi_t dev, uint8_t reg, char out, char *in)
{
    spi_transfer_byte(dev, reg, 0);
    return spi_transfer_byte(dev, out, in);
}

int spi_transfer_regs(spi_t dev, uint8_t reg, char *out, char *in, unsigned int length)
{
    spi_transfer_byte(dev, reg, 0);
    return spi_transfer_bytes(dev, out, in, length);
}

void spi_transmission_begin(spi_t dev, char reset_val)
{
    /* nothing to do here, as the NRF51822 does not support slave mode */
}

void spi_poweron(spi_t dev)
{
    if (dev < SPI_NUMOF) {
        spi[dev]->POWER = 1;
    }
}

void spi_poweroff(spi_t dev)
{
    if (dev < SPI_NUMOF) {
        spi[dev]->POWER = 0;
    }
}

#endif /* SPI_NUMOF */
