/*
 * Copyright (C) 2017 SKF AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup sys_auto_init_gnrc_netif
 * @{
 *
 * @file
 * @brief   Auto initialization for kw41zrf network interfaces
 *
 * @author  Joakim Nohlgård <joakim.nohlgard@eistec.se>
 */

#ifdef MODULE_KW41ZRF

#include "log.h"
#include "board.h"
#include "net/gnrc/netif/ieee802154.h"
#include "net/gnrc.h"

#include "kw41zrf.h"
#ifdef MODULE_CONTIKIMAC
#include "net/contikimac.h"
#endif /* MODULE_CONTIKIMAC */

/**
 * @name    Stack parameters for the MAC layer thread
 * @{
 */
#ifndef KW41ZRF_NETIF_STACKSIZE
#define KW41ZRF_NETIF_STACKSIZE     (THREAD_STACKSIZE_DEFAULT)
#endif
#ifndef KW41ZRF_NETIF_PRIO
#define KW41ZRF_NETIF_PRIO          (GNRC_NETIF_PRIO)
#endif
/** @} */

/* There is only one memory mapped transceiver in the supported SoCs, the driver
 * does not try to take into account multiple instances of the hardware module */
#define KW41ZRF_NUMOF 1

static kw41zrf_t kw41zrf_devs[KW41ZRF_NUMOF];
static char _kw41zrf_stacks[KW41ZRF_NUMOF][KW41ZRF_NETIF_STACKSIZE];
#ifdef MODULE_CONTIKIMAC
static contikimac_t contikimac_devs[KW41ZRF_NUMOF];
#endif /* MODULE_CONTIKIMAC */

void auto_init_kw41zrf(void)
{
    for (unsigned i = 0; i < KW41ZRF_NUMOF; i++) {
        LOG_DEBUG("[auto_init_netif] initializing kw41zrf #%u\n", i);
        kw41zrf_setup(&kw41zrf_devs[i]);
#if defined(MODULE_CONTIKIMAC)
        contikimac_setup(&contikimac_devs[i], &kw41zrf_devs[i].netdev.netdev);
        gnrc_netif_ieee802154_create(_kw41zrf_stacks[i], KW41ZRF_NETIF_STACKSIZE,
                                     KW41ZRF_NETIF_PRIO, "kw41zrf-contikimac",
                                     (netdev_t *)&contikimac_devs[i]);
#else
        gnrc_netif_ieee802154_create(_kw41zrf_stacks[i], KW41ZRF_NETIF_STACKSIZE,
                                     KW41ZRF_NETIF_PRIO, "kw41zrf",
                                     (netdev_t *)&kw41zrf_devs[i]);
#endif
    }
}
#else
typedef int dont_be_pedantic;
#endif /* MODULE_KW41ZRF */

/** @} */
