/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup  cpu_cortex-m3
 * @{
 *
 * @file
 * @brief       Basic definitions for the Cortex-M3 common module
 *
 * When ever you want to do something hardware related, that is accessing MCUs registers directly,
 * just include this file. It will then make sure that the MCU specific headers are included.
 *
 * @author      Stefan Pfeiffer <stefan.pfeiffer@fu-berlin.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Joakim Gebart <joakim.gebart@eistec.se>
 */

#ifndef __CORTEXM_COMMON_H
#define __CORTEXM_COMMON_H

/**
 * @brief Cortex-M3 has architecture specific atomic operations in atomic_arch.c.
 */
#define ARCH_HAS_ATOMIC_INT 1

#include "cpu-conf.h"

/**
 * For downwards compatibility with old RIOT code.
 * TODO: remove once core was adjusted
 */
#include "irq.h"

#ifdef __cplusplus
extern "C" {
#endif

#define eINT            enableIRQ
#define dINT            disableIRQ

/**
 * @brief Initialization of the CPU
 */
void cpu_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __CORTEXM_COMMON_H */
/** @} */
