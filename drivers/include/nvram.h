/*
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    nvram Non-volatile RAM
 * @ingroup     drivers
 * @brief       Non-volatile RAM interface
 *
 * This API is designed around non-volatile memories which do not need blockwise
 * erase, such as ferro-electric RAM (FRAM) or magneto-resistive RAM (MRAM).
 *
 * This interface is not suitable for flash memories.
 *
 * @{
 *
 * @file
 *
 * @brief       Generic non-volatile RAM driver interface
 * @author      Joakim Gebart <joakim.gebart@eistec.se>
 */

#ifndef DRIVERS_NVRAM_H_
#define DRIVERS_NVRAM_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration in order to declare function pointers which take this
 * type as a parameter within the struct. */
typedef struct nvram nvram_t;

/**
 * @brief Device descriptor for generic NVRAM devices.
 */
typedef struct nvram {
    /** @brief Pointer to device-specific read function */
    int (*read)(nvram_t *dev, uint8_t *dst, uint32_t src, size_t size);
    /** @brief Pointer to device-specific write function */
    int (*write)(nvram_t *dev, uint32_t dst, uint8_t *src, size_t size);
    /** @brief Device memory size */
    size_t size;
    /** @brief Device-specific parameters, if any. */
    void *extra;
} nvram_t;

/**
 * @brief Copy data from system memory to NVRAM.
 *
 * @param[in]  dev   Pointer to NVRAM device descriptor
 * @param[in]  dst   Starting address in the NVRAM device address space
 * @param[in]  src   Pointer to the first byte in the system memory address space
 * @param[in]  len   Number of bytes to copy
 *
 * @return          Number of bytes written on success
 * @return          <0 on errors
 */
int nvram_write(nvram_t *dev, uint32_t dst, uint8_t *src, size_t len);

/**
 * @brief Copy data from NVRAM to system memory.
 *
 * @param[in]  dev   Pointer to NVRAM device descriptor
 * @param[in]  dst   Pointer to the first byte in the system memory address space
 * @param[in]  src   Starting address in the NVRAM device address space
 * @param[in]  len   Number of bytes to copy
 *
 * @return          Number of bytes written on success
 * @return          <0 on errors
 */
int nvram_read(nvram_t *dev, uint8_t *dst, uint32_t src, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_NVRAM_H_ */
/** @} */
