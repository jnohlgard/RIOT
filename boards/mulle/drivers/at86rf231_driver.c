/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     board_mulle
 * @{
 *
 * @file        at86rf231_driver.c
 * @brief       Board specific implementations for the at86rf231 radio driver
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "cpu.h"
#include "sched.h"
#include "vtimer.h"

#include "arch/thread_arch.h"
#include "periph/gpio.h"
#include "periph/spi.h"
#include "periph_conf.h"

#include "at86rf231.h"
#include "at86rf231_spi.h"

uint8_t at86rf231_get_status(void)
{
    return at86rf231_reg_read(AT86RF231_REG__TRX_STATUS)
           & AT86RF231_TRX_STATUS_MASK__TRX_STATUS;
}

void at86rf231_spi_select(void)
{
    gpio_clear(AT86RF231_SS_GPIO);
}

void at86rf231_spi_unselect(void)
{
    gpio_set(AT86RF231_SS_GPIO);
}

void at86rf231_slp_set(void)
{
    gpio_set(AT86RF231_SLP_TR_GPIO);
}

void at86rf231_slp_clear(void)
{
    gpio_clear(AT86RF231_SLP_TR_GPIO);
}

//~ void at86rf231_rst_set(void)
//~ {
    //~ gpio_clear(AT86RF231_RST_GPIO);
//~ }
//~
//~ void at86rf231_rst_clear(void)
//~ {
    //~ gpio_set(AT86RF231_RST_GPIO);
//~ }

void at86rf231_enable_interrupts(void)
{
    gpio_irq_enable(AT86RF231_IRQ_GPIO);
}

void at86rf231_disable_interrupts(void)
{
    gpio_irq_disable(AT86RF231_IRQ_GPIO);
}

void at86rf231_gpio_spi_interrupts_init(void)
{
    /* SPI init */
    spi_init_master(AT86RF231_SPI, SPI_CONF_FIRST_RISING, SPI_SPEED_5MHZ);

    spi_poweron(AT86RF231_SPI);

    /* IRQ0 */
    gpio_init_in(AT86RF231_IRQ_GPIO, GPIO_NOPULL);
    gpio_init_int(AT86RF231_IRQ_GPIO, GPIO_NOPULL, GPIO_RISING, (gpio_cb_t)at86rf231_rx_irq, NULL);

    /* Connect EXTI4 Line to PC4 pin */
    at86rf231_enable_interrupts();

    /* CS */
    gpio_init_out(AT86RF231_SS_GPIO, GPIO_NOPULL);
    /* SLEEP */
    gpio_init_out(AT86RF231_SLP_TR_GPIO, GPIO_NOPULL);
    /* RESET */
    //~ gpio_init_out(AT86RF231_RST_GPIO, GPIO_NOPULL);

}

void at86rf231_reset(void)
{
    /* force reset */
    //~ at86rf231_rst_set();

    /* put pins to default values */
    at86rf231_spi_unselect();
    at86rf231_slp_clear();

    /* additional waiting to comply to min rst pulse width */
    uint8_t delay = 50;
    while (delay--){}

    at86rf231_rst_clear();

    /* Send a FORCE TRX OFF command */
    at86rf231_reg_write(AT86RF231_REG__TRX_STATE, AT86RF231_TRX_STATE__FORCE_TRX_OFF);

    /* busy wait for TRX_OFF state */
    uint8_t status;
    uint8_t max_wait = 100;

    do {
        status = at86rf231_get_status();

        if (!--max_wait) {
            printf("at86rf231 : ERROR : could not enter TRX_OFF mode\n");
            break;
        }
    } while ((status & AT86RF231_TRX_STATUS_MASK__TRX_STATUS)
             != AT86RF231_TRX_STATUS__TRX_OFF);
}

uint8_t at86rf231_spi_transfer_byte(uint8_t byte)
{
    char ret;
    spi_transfer_byte(AT86RF231_SPI, byte, &ret);
    return ret;
}

void at86rf231_spi_transfer(const uint8_t *data_out, uint8_t *data_in, uint16_t length)
{
    spi_transfer_bytes(AT86RF231_SPI, (char*)data_out, (char*)data_in, length);
}
