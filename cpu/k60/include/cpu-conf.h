/*
 * Copyright (C) 2014 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup        cpu_k60 K60
 * @ingroup         cpu
 * @brief           CPU specific implementations for the K60
 * @{
 *
 * @file
 * @brief           Implementation specific CPU configuration options
 *
 * @author          Joakim Gebart <joakim.gebart@eistec.se>
 */

#ifndef __CPU_CONF_H
#define __CPU_CONF_H

#if defined(CPU_MODEL_K60DN512VLL10) || defined(CPU_MODEL_K60DN256VLL10)

/* Rev. 2.x silicon */
#define K60_CPU_REV 2
#include "MK60D10.h"

/** The expected CPUID value, can be used to implement a check that we are
 * running on the right hardware */
#define K60_EXPECTED_CPUID 0x410fc241u

#elif defined(CPU_MODEL_K60DN512ZVLL10) || defined(CPU_MODEL_K60DN256ZVLL10)

/* Rev. 1.x silicon */
#define K60_CPU_REV 1
#include "MK60DZ10.h"

/** The expected CPUID value, can be used to implement a check that we are
 * running on the right hardware */
#define K60_EXPECTED_CPUID 0x410fc240u

/* Some compatibility defines to minimize the ifdefs needed for the register
 * name changes */

#define SIM_SCGC6_SPI0_MASK SIM_SCGC6_DSPI0_MASK
#define SIM_SCGC6_SPI0_SHIFT SIM_SCGC6_DSPI0_SHIFT

#define MCG_C2_RANGE0_MASK MCG_C2_RANGE_MASK
#define MCG_C5_PRDIV0_MASK MCG_C5_PRDIV_MASK
#define MCG_C6_VDIV0_MASK MCG_C6_VDIV_MASK

#define UART_BASES                               { UART0, UART1, UART2, UART3, UART4, UART5 }

#else
#error Unknown CPU model. Update Makefile.include in the board directory.
#endif

/* Compatibility defines for compatibility with differing module names in MKW2x */
#define SIM_SCGC5_LPTMR_MASK SIM_SCGC5_LPTIMER_MASK
#define SIM_SCGC5_LPTMR_SHIFT SIM_SCGC5_LPTIMER_SHIFT

/* Helpers for the below macro, used to expand the sig variable to a numeric
 * string, even if it is defined as a macro constant. */
#define K60_H_EXPAND_AND_STRINGIFY(s) K60_H_STRINGIFY(s)
#define K60_H_STRINGIFY(s) #s

/**
 * Make the CPU signal to the debugger and break execution by issuing a bkpt
 * instruction.
 */
#define DEBUGGER_BREAK(sig) asm volatile ("bkpt #" K60_H_EXPAND_AND_STRINGIFY(sig) "\n")

/*
 * These are constants that can be used to identify the signal to
 * DEBUGGER_BREAK(). These can not be converted to an enum because
 * DEBUGGER_BREAK() is a macro and not a function
 */
#define BREAK_FAULT_HANDLER 0
#define BREAK_INVALID_PARAM 1
#define BREAK_EXIT 2
#define BREAK_MEMORY_CORRUPTION 3
#define BREAK_WRONG_K60_CPU_REV 99
/* Do not use 0xAB, it is reserved for ARM semihosting environment. */
#define BREAK_SEMIHOSTING 0xAB

#define PIN_MUX_FUNCTION_ANALOG 0
#define PIN_MUX_FUNCTION_GPIO 1
#define PIN_INTERRUPT_RISING 0b1001
#define PIN_INTERRUPT_FALLING 0b1010
#define PIN_INTERRUPT_EDGE 0b1011

/**
 * @name Kernel configuration
 * @{
 */
#define KERNEL_CONF_STACKSIZE_PRINTF    (2500)

#ifndef KERNEL_CONF_STACKSIZE_DEFAULT
#define KERNEL_CONF_STACKSIZE_DEFAULT   (2500)
#endif

#define KERNEL_CONF_STACKSIZE_IDLE      (512)
/** @} */

/**
 * @name Length for reading CPU_ID (named UID in Freescale documents)
 */
#define CPUID_ID_LEN                    (16)
#define CPUID_ID_PTR                    ((void *)(&(SIM->UIDH)))


/**
 * @name UART0 buffer size definition for compatibility reasons
 *
 * TODO: remove once the remodeling of the uart0 driver is done
 * @{
 */
#ifndef UART0_BUFSIZE
#define UART0_BUFSIZE                   (128)
#endif
/** @} */

/**
 * @name Clock settings for the lptmr timer
 */
#define LPTIMER_CLKSRC_MCGIRCLK          0    /**< internal reference clock (4MHz) */
#define LPTIMER_CLKSRC_LPO               1    /**< PMC 1kHz output */
#define LPTIMER_CLKSRC_ERCLK32K          2    /**< RTC clock 32768Hz */
#define LPTIMER_CLKSRC_OSCERCLK          3    /**< system oscillator output, clock from RF-Part */

#ifndef LPTIMER_CLKSRC
#define LPTIMER_CLKSRC                   LPTIMER_CLKSRC_LPO    /**< default clock source */
#endif

#if (LPTIMER_CLKSRC == LPTIMER_CLKSRC_MCGIRCLK)
#define LPTIMER_CLK_PRESCALE    1
#define LPTIMER_SPEED           1000000
#elif (LPTIMER_CLKSRC == LPTIMER_CLKSRC_OSCERCLK)
#define LPTIMER_CLK_PRESCALE    1
#define LPTIMER_SPEED           1000000
#elif (LPTIMER_CLKSRC == LPTIMER_CLKSRC_ERCLK32K)
#define LPTIMER_CLK_PRESCALE    0
#define LPTIMER_SPEED           32768
#else
#define LPTIMER_CLK_PRESCALE    0
#define LPTIMER_SPEED           1000
#endif


#endif /* __CPU_CONF_H */
/** @} */
