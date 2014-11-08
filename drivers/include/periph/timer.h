/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    driver_periph_timer Timer
 * @ingroup     driver_periph
 * @brief       Low-level timer peripheral driver
 * @{
 *
 * @file
 * @brief       Low-level timer peripheral driver interface definitions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef __TIMER_H
#define __TIMER_H

#include <stdint.h>

#include "periph_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* guard file in case no timer is defined */
#if TIMER_0_EN | TIMER_1_EN | TIMER_2_EN

/**
 * @brief Definition of available timers
 *
 * Each timer is based on a hardware timer, which can further have 1 or more channels.
 * To this point 4 timers are possible, might need to be expanded for some cases.
 */
typedef enum {
#if TIMER_0_EN
    TIMER_0 = 0,            /**< 1st timer */
#endif
#if TIMER_1_EN
    TIMER_1,                /**< 2nd timer */
#endif
#if TIMER_2_EN
    TIMER_2,                /**< 3rd timer */
#endif
#if TIMER_3_EN
    TIMER_3,                /**< 4th timer */
#endif
} periph_timer_t; /* named tim instead of timer to avoid conflicts with vendor libraries */

typedef void(*timer_cb_t)(void *arg);

/**
 * @brief Initialize the given timer
 *
 * Each timer device is running with the given speed. Each can contain one or more channels
 * as defined in periph_conf.h. The timer is configured in up-counting mode and will count
 * until TIMER_x_MAX_VALUE as defined in used board's periph_conf.h until overflowing.
 *
 * The timer will be started automatically after initialization with interrupts enabled.
 *
 * @param[in] dev           the timer to initialize
 *
 * @return                  returns 0 on success,
 * @return                  -1 on undefined device given
 */
int timer_init(periph_timer_t dev);

/**
 * @brief [brief description]
 *
 * @note Call this only after you have called *timer_init*.
 *
 * @param dev [description]
 * @param callback [description]
 *
 * @return [description]
 */
int timer_on_overflow(periph_timer_t dev, timer_cb_t cb, void *arg);

/**
 * @brief Set a given timer channel for the given timer device. The callback given during
 * initialization is called when timeout ticks have passed after calling this function
 *
 * @param[in] dev           the timer device to set
 * @param[in] channel       the channel to set
 * @param[in] timeout       timeout in ticks after that the registered callback is executed
 *
 * @return                  1 on success, -1 on error
 */
int timer_set_rel(periph_timer_t dev, uint8_t chan, unsigned int timeout, timer_cb_t cb, void *arg);

/**
 * @brief Set an absolute timeout value for the given channel of the given timer device
 *
 * @param[in] dev           the timer device to set
 * @param[in] channel       the channel to set
 * @param[in] value         the absolute compare value when the callback will be triggered
 *
 * @return                  1 on success, -1 on error
 */
int timer_set_abs(periph_timer_t dev, uint8_t chan, unsigned int value, timer_cb_t cb, void *arg);

/**
 * @brief Clear the given channel of the given timer device
 *
 * @param[in] dev           the timer device to clear
 * @param[in] channel       the channel on the given device to clear
 *
 * @return                  1 on success, -1 on error
 */
int timer_clear(periph_timer_t dev, int channel);

/**
 * @brief Read the current value of the given timer device
 *
 * @param[in] dev           the timer to read the current value from
 *
 * @return                  the timers current value
 */
int timer_read(periph_timer_t dev, uint32_t *value);

/**
 * @brief Start the given timer. This function is only needed if the timer was stopped manually before
 *
 * @param[in] dev           the timer device to stop
 */
void timer_start(periph_timer_t dev);

/**
 * @brief Stop the given timer - this will effect all of the timer's channels
 *
 * @param[in] dev           the timer to stop
 */
void timer_stop(periph_timer_t dev);

/**
 * @brief Enable the interrupts for the given timer
 *
 * @param[in] dev           timer to enable interrupts for
 */
void timer_irq_enable(periph_timer_t dev);

/**
 * @brief Disable interrupts for the given timer
 *
 * @param[in] dev           the timer to disable interrupts for
 */
void timer_irq_disable(periph_timer_t dev);

/**
 * @brief Reset the up-counting value to zero for the given timer
 *
 * Note that this function effects all currently set channels and it can lead to non-deterministic timeouts
 * if any channel is active when this function is called.
 *
 * @param[in] dev           the timer to reset
 */
void timer_reset(periph_timer_t dev);

#endif /* TIMER_0_EN | TIMER_1_EN | TIMER_2_EN */

#ifdef __cplusplus
}
#endif

#endif /* __TIMER_H */
/** @} */
