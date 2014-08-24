/*
 * Copyright (c) 2014, Eistec AB.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Device I/O helpers for UARTs on K60, implementation.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include <string.h>
#include "devio-uart.h"
#include "periph/uart.h"
#include "cpu.h"

static inline long uart_write_r(uart_t uart_num, struct _reent *r, int fd, const char *ptr, int len) {
  int i = 0;
  while (i < len) {
    uart_write_blocking(uart_num, ptr[i]);
    ++i;
  }
  return i;
}

static long uart_read_r (uart_t uart_num, struct _reent *r, int fd, char *ptr, int len) {
  /* not yet implemented */
  return 0;
}

#if UART_0_EN
long uart0_write_r(struct _reent *r, int fd, const char *ptr, int len) {
  return uart_write_r(UART_0, r, fd, ptr, len);
}
#endif

#if UART_1_EN
long uart1_write_r(struct _reent *r, int fd, const char *ptr, int len) {
  return uart_write_r(UART_1, r, fd, ptr, len);
}
#endif

#if UART_2_EN
long uart2_write_r(struct _reent *r, int fd, const char *ptr, int len) {
  return uart_write_r(UART_2, r, fd, ptr, len);
}
#endif

#if UART_3_EN
long uart3_write_r(struct _reent *r, int fd, const char *ptr, int len) {
  return uart_write_r(UART_3, r, fd, ptr, len);
}
#endif

#if UART_4_EN
long uart4_write_r(struct _reent *r, int fd, const char *ptr, int len) {
  return uart_write_r(UART_4, r, fd, ptr, len);
}
#endif

#if UART_0_EN
long uart0_read_r (struct _reent *r, int fd, char *ptr, int len) {
  return uart_read_r(UART_0, r, fd, ptr, len);
}
#endif


