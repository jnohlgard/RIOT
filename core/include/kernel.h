/*
 * Copyright (C) 2013 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @addtogroup  core_internal
 * @{
 *
 * @file        kernel.h
 * @brief       Kernel compile time configuration
 *
 * A reboot() function is also provided (and used by core_panic() when needed).
 *
 * @author      Freie Universität Berlin, Computer Systems & Telematics
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdbool.h>

#include "attributes.h"
#include "config.h"
#include "tcb.h"
#include "flags.h"
#include "sched.h"

#include "cpu-conf.h"

#ifndef CPU_ID_LEN
/**
 * @brief Length of cpu_id_t type. Override in cpu-conf.h of CPU.
 */
#define CPU_ID_LEN  (2U)
#endif

/**
 * @brief Data type to represent the CPU's unique serial number.
 */
typedef struct {
    unsigned char id[CPU_ID_LEN];
} cpu_id_t;

#include "cpu.h"

/**
 * @def KERNEL_CONF_STACKSIZE_DEFAULT
 * @brief A reasonable default stack size that will suffice most smaller tasks
 */
#ifndef KERNEL_CONF_STACKSIZE_DEFAULT
#error KERNEL_CONF_STACKSIZE_DEFAULT must be defined per CPU
#endif

/**
 * @def KERNEL_CONF_STACKSIZE_IDLE
 * @brief Size of the idle task's stack in bytes
 */
#ifndef KERNEL_CONF_STACKSIZE_IDLE
#error KERNEL_CONF_STACKSIZE_IDLE must be defined per CPU
#endif

/**
 * @def KERNEL_CONF_STACKSIZE_PRINTF
 * @ingroup conf
 * @brief Size of the task's printf stack in bytes
 */
#ifndef KERNEL_CONF_STACKSIZE_PRINTF
#error KERNEL_CONF_STACKSIZE_PRINTF must be defined per CPU
#endif

/**
 * @def KERNEL_CONF_STACKSIZE_MAIN
 * @brief Size of the main task's stack in bytes
 */
#ifndef KERNEL_CONF_STACKSIZE_MAIN
#define KERNEL_CONF_STACKSIZE_MAIN      (KERNEL_CONF_STACKSIZE_DEFAULT + KERNEL_CONF_STACKSIZE_PRINTF)
#endif

/* ------------------------------------------------------------------------- */


#define PID_NULL -1

#define PRIORITY_MIN            SCHED_PRIO_LEVELS-1

#define PRIORITY_IDLE           PRIORITY_MIN
#define PRIORITY_MAIN           (PRIORITY_MIN - (SCHED_PRIO_LEVELS/2))

#define LPM_PREVENT_SLEEP_UART    BIT2
#define LPM_PREVENT_SLEEP_HWTIMER    BIT1

#ifndef GET_CPU_ID
/**
 * @brief Macro to get CPU's unique serial number. Redefine in cpu.h.
 *
 * @param[out] id   A cpu_id_t variable that the CPU can fill.
 */
#define GET_CPU_ID(id)  for (int i = 0; i < CPU_ID_LEN; id.id[i] = 0xff, i++)
#endif

extern volatile int lpm_prevent_sleep;

extern config_t sysconfig;

/* ------------------------------------------------------------------------- */

/**
 * @brief Immediately reboots the system.
 *
 * This function is used by core_panic() when the DEVELHELP macro is not defined.
 *
 * @return WARNING: this function NEVER returns!
 */
NORETURN void reboot(void);

/**
 * @brief Returns the serial number of the CPU.
 *
 * @param[out] id           Pointer to a buffer where the serial number gets
 *                          copied into, will be truncated if *id_len* is to
 *                          short
 * @param[in,out] id_len    Length of the CPU ID in bytes.
 *
 * @return  *id* on success, NULL if CPU does not defines the macros
 *          CPU_ID_ADDR and CPU_ID_LEN or CPU_ID(*id*, *id_len*) and
 *          CPU_ID_LEN.
 */
unsigned char *cpu_id(unsigned char *id, int *id_len);

/** @} */
#endif /* KERNEL_H_ */
