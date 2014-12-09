/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     board_mbed_lpc1768
 * @{
 *
 * @file
 * @brief       Peripheral MCU configuration for the mbed LPC1768 board
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef __PERIPH_CONF_H
#define __PERIPH_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Timer configuration
 * @{
 */
#define TIMER_NUMOF         (1U)
#define TIMER_0_EN          1
#define TIMER_IRQ_PRIO      1

/* Timer 0 configuration */
#define TIMER_0_DEV         LPC_TIM0
#define TIMER_0_CHANNELS    4
#define TIMER_0_PRESCALER   (95U)
#define TIMER_0_MAX_VALUE   (0xffffffff)
#define TIMER_0_CLKEN()     (LPC_SC->PCONP |= (1 << 1))
#define TIMER_0_CLKDIS()    (LPC_SC->PCONP &= ~(1 << 1))
#define TIMER_0_PLKSEL()    (LPC_SC->PCLKSEL0 |= (1 << 2))
#define TIMER_0_ISR         isr_timer0
#define TIMER_0_IRQ         TIMER0_IRQn
/** @} */

/**
 * @brief UART configuration
 * @{
 */
#define UART_NUMOF          (1U)
#define UART_0_EN           1
#define UART_1_EN           0
#define UART_IRQ_PRIO       1

/* UART 0 device configuration */
#define UART_0_DEV          LPC_UART3
#define UART_0_CLKEN()      (LPC_SC->PCONP |= (1 << 25))
#define UART_0_CLKDIS()     (LPC_SC->PCONP &= ~(1 << 25))
#define UART_0_IRQ          UART3_IRQn
#define UART_0_ISR          isr_uart3
/* UART 0 pin configuration */
#define UART_0_PINSEL       (LPC_PINCON->PINSEL0)
#define UART_0_PINMODE      (LPC_PINCON->PINMODE0)
#define UART_0_RX_PIN       (0)
#define UART_0_TX_PIN       (1)
#define UART_0_AF           (2)
/** @} */

/**
 * @brief GPIO configuration
 * @{
 */
#define GPIO_NUMOF          (8U)
#define GPIO_0_EN           1
#define GPIO_1_EN           1
#define GPIO_2_EN           1
#define GPIO_3_EN           1
#define GPIO_4_EN           1
#define GPIO_5_EN           1
#define GPIO_6_EN           1
#define GPIO_7_EN           1
#define GPIO_IRQ_PRIO       1

/* GPIO device configuration */
#define GPIO_0_DEV          LPC_GPIO0
#define GPIO_0_PIN          1
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __PERIPH_CONF_H */
/** @} */
