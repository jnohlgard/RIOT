/*
 * Copyright (C) 2017 SKF AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_kw41zrf
 * @{
 *
 * @file
 * @brief       Netdev interface for kw41zrf drivers
 *
 * @author      Joakim Nohlgård <joakim.nohlgard@eistec.se>
 */

#include <string.h>
#include <assert.h>
#include <errno.h>

#include "log.h"
#include "net/eui64.h"
#include "net/ieee802154.h"
#include "net/netdev.h"
#include "net/netdev/ieee802154.h"

#include "kw41zrf.h"
#include "kw41zrf_netdev.h"
#include "kw41zrf_intern.h"
#include "kw41zrf_getset.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define _MAX_MHR_OVERHEAD           (25)

#define _MACACKWAITDURATION         (864 / 16) /* 864us * 62500Hz */

static void kw41zrf_irq_handler(void *arg)
{
    netdev_t *dev = (netdev_t *) arg;

    if (dev->event_callback) {
        dev->event_callback(dev, NETDEV_EVENT_ISR);
    }
}

static int kw41zrf_netdev_init(netdev_t *netdev)
{
    kw41zrf_t *dev = (kw41zrf_t *)netdev;

    /* initialise SPI and GPIOs */
    if (kw41zrf_init(dev, &kw41zrf_irq_handler)) {
        LOG_ERROR("[kw41zrf] unable to initialize device\n");
        return -1;
    }

#ifdef MODULE_NETSTATS_L2
    memset(&netdev->stats, 0, sizeof(netstats_t));
#endif

    /* reset device to default values and put it into RX state */
    kw41zrf_reset_phy(dev);

    return 0;
}

static inline size_t kw41zrf_tx_load(const void *buf, size_t len, size_t offset)
{
    /* Array bounds are checked in the kw41zrf_netdev_send loop */
    /* offset + 1 is used because buf[0] contains the frame length byte */
    memcpy(((uint8_t *)&ZLL->PKT_BUFFER_TX[0]) + offset + 1, buf, len);
    return offset + len;
}

static void kw41zrf_tx_exec(kw41zrf_t *dev)
{
    /* Check FCF field in the TX buffer to see if the ACK_REQ flag was set in
     * the packet that is queued for transmission */
    uint8_t fcf = *(((volatile uint8_t *)&ZLL->PKT_BUFFER_TX[0]) + 1);
    if ((dev->netdev.flags & KW41ZRF_OPT_AUTOACK) &&
        (fcf & IEEE802154_FCF_ACK_REQ)) {
        kw41zrf_set_sequence(dev, XCVSEQ_TX_RX);
    }
    else {
        kw41zrf_set_sequence(dev, XCVSEQ_TRANSMIT);
    }
}

static int kw41zrf_netdev_send(netdev_t *netdev, const struct iovec *vector, unsigned count)
{
    kw41zrf_t *dev = (kw41zrf_t *)netdev;
    const struct iovec *ptr = vector;
    size_t len = 0;

    /* load packet data into buffer */
    for (unsigned i = 0; i < count; i++, ptr++) {
        /* current packet data + FCS too long */
        if ((len + ptr->iov_len + IEEE802154_FCS_LEN) > KW41ZRF_MAX_PKT_LENGTH) {
            LOG_ERROR("[kw41zrf] packet too large (%u byte) to fit\n",
                  (unsigned)len + IEEE802154_FCS_LEN);
            return -EOVERFLOW;
        }
        len = kw41zrf_tx_load(ptr->iov_base, ptr->iov_len, len);
    }

    /* make sure ongoing T or TR sequence are finished */
    if (kw41zrf_can_switch_to_idle(dev)) {
        kw41zrf_set_sequence(dev, XCVSEQ_IDLE);
        dev->pending_tx++;
    }
    else {
        /* do not wait, this can lead to a dead lock */
        return 0;
    }

    /*
     * First octet in the TX buffer contains the frame length.
     * Nbytes = FRAME_LEN - 2 -> FRAME_LEN = Nbytes + 2
     * MKW41Z ref. man. 44.6.2.6.3.1.3 Sequence T (Transmit), p. 2147
     */
    *((volatile uint8_t *)&ZLL->PKT_BUFFER_TX[0]) = len + IEEE802154_FCS_LEN;

#ifdef MODULE_NETSTATS_L2
    netdev->stats.tx_bytes += len;
#endif

    /* send data out directly if pre-loading is disabled */
    if (!(dev->netdev.flags & KW41ZRF_OPT_PRELOADING)) {
        kw41zrf_tx_exec(dev);
    }

    return (int)len;
}

static int kw41zrf_netdev_recv(netdev_t *netdev, void *buf, size_t len, void *info)
{
    /* get size of the received packet */
    size_t pkt_len = (ZLL->IRQSTS & ZLL_IRQSTS_RX_FRAME_LENGTH_MASK) >> ZLL_IRQSTS_RX_FRAME_LENGTH_SHIFT;

    /* just return length when buf == NULL */
    if (buf == NULL) {
        return pkt_len;
    }

#ifdef MODULE_NETSTATS_L2
    netdev->stats.rx_count++;
    netdev->stats.rx_bytes += pkt_len;
#else
    (void)netdev;
#endif

    if (pkt_len > len) {
        /* not enough space in buf */
        return -ENOBUFS;
    }

    /* Copy the packet, skip the frame length byte */
    memcpy((uint8_t *)buf, ((uint8_t*)&ZLL->PKT_BUFFER_RX[0]) + 1, pkt_len);

    if (info != NULL) {
        netdev_ieee802154_rx_info_t *radio_info = info;
        radio_info->lqi = (ZLL->LQI_AND_RSSI & ZLL_LQI_AND_RSSI_LQI_VALUE_MASK) >> ZLL_LQI_AND_RSSI_LQI_VALUE_SHIFT;
        radio_info->rssi = (ZLL->LQI_AND_RSSI & ZLL_LQI_AND_RSSI_RSSI_MASK) >> ZLL_LQI_AND_RSSI_RSSI_SHIFT;
    }

    /* skip PHR and FCS */
    return pkt_len - 2;
}

static int kw41zrf_netdev_set_state(kw41zrf_t *dev, netopt_state_t state)
{
    switch (state) {
        case NETOPT_STATE_SLEEP:
//             kw41zrf_set_power_mode(dev, KW41ZRF_DOZE);
            break;
        case NETOPT_STATE_IDLE:
//             kw41zrf_set_power_mode(dev, KW41ZRF_AUTODOZE);
            kw41zrf_set_sequence(dev, dev->idle_state);
            break;
        case NETOPT_STATE_TX:
            if (dev->netdev.flags & KW41ZRF_OPT_PRELOADING) {
                kw41zrf_tx_exec(dev);
            }
            break;
        case NETOPT_STATE_RESET:
            kw41zrf_reset_phy(dev);
            break;
        case NETOPT_STATE_OFF:
            /* TODO: Replace with powerdown (set reset input low) */
//             kw41zrf_set_power_mode(dev, KW41ZRF_HIBERNATE);
        default:
            return -ENOTSUP;
    }
    return sizeof(netopt_state_t);
}

int kw41zrf_netdev_get(netdev_t *netdev, netopt_t opt, void *value, size_t len)
{
    kw41zrf_t *dev = (kw41zrf_t *)netdev;

    if (dev == NULL) {
        return -ENODEV;
    }

    switch (opt) {
        case NETOPT_MAX_PACKET_SIZE:
            if (len < sizeof(int16_t)) {
                return -EOVERFLOW;
            }

            *((uint16_t *)value) = KW41ZRF_MAX_PKT_LENGTH - _MAX_MHR_OVERHEAD;
            return sizeof(uint16_t);

        case NETOPT_STATE:
            if (len < sizeof(netopt_state_t)) {
                return -EOVERFLOW;
            }
            *((netopt_state_t *)value) = dev->state;
            return sizeof(netopt_state_t);

        case NETOPT_PRELOADING:
            if (dev->netdev.flags & KW41ZRF_OPT_PRELOADING) {
                *((netopt_enable_t *)value) = NETOPT_ENABLE;
            }
            else {
                *((netopt_enable_t *)value) = NETOPT_DISABLE;
            }
            return sizeof(netopt_enable_t);

        case NETOPT_PROMISCUOUSMODE:
            if (dev->netdev.flags & KW41ZRF_OPT_PROMISCUOUS) {
                *((netopt_enable_t *)value) = NETOPT_ENABLE;
            }
            else {
                *((netopt_enable_t *)value) = NETOPT_DISABLE;
            }
            return sizeof(netopt_enable_t);

        case NETOPT_RX_START_IRQ:
            *((netopt_enable_t *)value) =
                !!(dev->netdev.flags & KW41ZRF_OPT_TELL_RX_START);
            return sizeof(netopt_enable_t);

        case NETOPT_RX_END_IRQ:
            *((netopt_enable_t *)value) =
                !!(dev->netdev.flags & KW41ZRF_OPT_TELL_RX_END);
            return sizeof(netopt_enable_t);

        case NETOPT_TX_START_IRQ:
            *((netopt_enable_t *)value) =
                !!(dev->netdev.flags & KW41ZRF_OPT_TELL_TX_START);
            return sizeof(netopt_enable_t);

        case NETOPT_TX_END_IRQ:
            *((netopt_enable_t *)value) =
                !!(dev->netdev.flags & KW41ZRF_OPT_TELL_TX_END);
            return sizeof(netopt_enable_t);

        case NETOPT_AUTOCCA:
            *((netopt_enable_t *)value) =
                !!(dev->netdev.flags & KW41ZRF_OPT_AUTOCCA);
            return sizeof(netopt_enable_t);

        case NETOPT_TX_POWER:
            if (len < sizeof(int16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)value) = kw41zrf_get_txpower(dev);
            return sizeof(uint16_t);

        case NETOPT_IS_CHANNEL_CLR:
            if (kw41zrf_cca(dev)) {
                *((netopt_enable_t *)value) = NETOPT_ENABLE;
            }
            else {
                *((netopt_enable_t *)value) = NETOPT_DISABLE;
            }
            return sizeof(netopt_enable_t);

        case NETOPT_CCA_THRESHOLD:
            if (len < sizeof(uint8_t)) {
                return -EOVERFLOW;
            }
            else {
                *(int8_t *)value = kw41zrf_get_cca_threshold(dev);
            }
            return sizeof(int8_t);

        case NETOPT_CCA_MODE:
            if (len < sizeof(uint8_t)) {
                return -EOVERFLOW;
            }
            else {
                *(uint8_t *)value = kw41zrf_get_cca_mode(dev);
                switch (*((int8_t *)value)) {
                    case NETDEV_IEEE802154_CCA_MODE_1:
                    case NETDEV_IEEE802154_CCA_MODE_2:
                    case NETDEV_IEEE802154_CCA_MODE_3:
                        return sizeof(uint8_t);
                    default:
                        break;
                }
                return -EOVERFLOW;
            }
            break;

        case NETOPT_CHANNEL_PAGE:
        default:
            break;
    }

    return netdev_ieee802154_get((netdev_ieee802154_t *)netdev, opt, value, len);
}

static int kw41zrf_netdev_set(netdev_t *netdev, netopt_t opt, void *value, size_t len)
{
    kw41zrf_t *dev = (kw41zrf_t *)netdev;
    int res = -ENOTSUP;

    if (dev == NULL) {
        return -ENODEV;
    }

    switch (opt) {
        case NETOPT_ADDRESS:
            if (len > sizeof(uint16_t)) {
                res = -EOVERFLOW;
            }
            else {
                kw41zrf_set_addr_short(dev, *((uint16_t *)value));
                /* don't set res to set netdev_ieee802154_t::short_addr */
            }
            break;

        case NETOPT_ADDRESS_LONG:
            if (len > sizeof(uint64_t)) {
                return -EOVERFLOW;
            }
            else {
                kw41zrf_set_addr_long(dev, *((uint64_t *)value));
                /* don't set res to set netdev_ieee802154_t::short_addr */
            }
            break;

        case NETOPT_NID:
            if (len > sizeof(uint16_t)) {
                return -EOVERFLOW;
            }

            else {
                kw41zrf_set_pan(dev, *((uint16_t *)value));
                /* don't set res to set netdev_ieee802154_t::pan */
            }
            break;

        case NETOPT_CHANNEL:
            if (len != sizeof(uint16_t)) {
                res = -EINVAL;
            }
            else {
                uint8_t chan = ((uint8_t *)value)[0];
                if (kw41zrf_set_channel(dev, chan)) {
                    res = -EINVAL;
                    break;
                }
                dev->netdev.chan = chan;
                /* don't set res to set netdev_ieee802154_t::chan */
            }
            break;

        case NETOPT_CHANNEL_PAGE:
            res = -EINVAL;
            break;

        case NETOPT_TX_POWER:
            if (len < sizeof(uint16_t)) {
                res = -EOVERFLOW;
            }
            else {
                kw41zrf_set_tx_power(dev, *(int16_t *)value);
                res = sizeof(uint16_t);
            }
            break;

        case NETOPT_STATE:
            if (len > sizeof(netopt_state_t)) {
                res = -EOVERFLOW;
            }
            else {
                res = kw41zrf_netdev_set_state(dev, *((netopt_state_t *)value));
            }
            break;

        case NETOPT_AUTOACK:
            /* Set up HW generated automatic ACK after Receive */
            kw41zrf_set_option(dev, KW41ZRF_OPT_AUTOACK,
                              ((bool *)value)[0]);
            break;

        case NETOPT_ACK_REQ:
            kw41zrf_set_option(dev, KW41ZRF_OPT_ACK_REQ,
                              ((bool *)value)[0]);
            break;

        case NETOPT_PRELOADING:
            kw41zrf_set_option(dev, KW41ZRF_OPT_PRELOADING,
                              ((bool *)value)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_PROMISCUOUSMODE:
            kw41zrf_set_option(dev, KW41ZRF_OPT_PROMISCUOUS,
                              ((bool *)value)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_RX_START_IRQ:
            kw41zrf_set_option(dev, KW41ZRF_OPT_TELL_RX_START,
                                 ((bool *)value)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_RX_END_IRQ:
            kw41zrf_set_option(dev, KW41ZRF_OPT_TELL_RX_END,
                                 ((bool *)value)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_TX_START_IRQ:
            kw41zrf_set_option(dev, KW41ZRF_OPT_TELL_TX_START,
                                 ((bool *)value)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_TX_END_IRQ:
            kw41zrf_set_option(dev, KW41ZRF_OPT_TELL_TX_END,
                                 ((bool *)value)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_AUTOCCA:
            kw41zrf_set_option(dev, KW41ZRF_OPT_AUTOCCA,
                                 ((bool *)value)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_CCA_THRESHOLD:
            if (len < sizeof(uint8_t)) {
                res = -EOVERFLOW;
            }
            else {
                kw41zrf_set_cca_threshold(dev, *((int8_t*)value));
                res = sizeof(uint8_t);
            }
            break;

        case NETOPT_CCA_MODE:
            if (len < sizeof(uint8_t)) {
                res = -EOVERFLOW;
            }
            else {
                switch (*((int8_t*)value)) {
                    case NETDEV_IEEE802154_CCA_MODE_1:
                    case NETDEV_IEEE802154_CCA_MODE_2:
                    case NETDEV_IEEE802154_CCA_MODE_3:
                        kw41zrf_set_cca_mode(dev, *((int8_t*)value));
                        res = sizeof(uint8_t);
                        break;
                    case NETDEV_IEEE802154_CCA_MODE_4:
                    case NETDEV_IEEE802154_CCA_MODE_5:
                    case NETDEV_IEEE802154_CCA_MODE_6:
                    default:
                        break;
                }
            }
            break;

        case NETOPT_RF_TESTMODE:
#ifdef KW41ZRF_TESTMODE
            if (len < sizeof(uint8_t)) {
                res = -EOVERFLOW;
            }
            else {
                kw41zrf_set_test_mode(dev, *((uint8_t *)value));
                res = sizeof(uint8_t);
            }
#endif
            break;

        default:
            break;
    }

    if (res == -ENOTSUP) {
        res = netdev_ieee802154_set((netdev_ieee802154_t *)netdev, opt,
                                     value, len);
    }

    return res;
}

static void _isr_event_seq_r(netdev_t *netdev)
{
    kw41zrf_t *dev = (kw41zrf_t *)netdev;
    uint32_t irqsts = ZLL->IRQSTS;
    uint32_t handled_irqs = 0;

    if (irqsts & ZLL_IRQSTS_RXWTRMRKIRQ_MASK) {
        DEBUG("[kw41zrf] RXWTRMRKIRQ (R)\n");
        handled_irqs |= ZLL_IRQSTS_RXWTRMRKIRQ_MASK;
        netdev->event_callback(netdev, NETDEV_EVENT_RX_STARTED);
    }

    if (irqsts & ZLL_IRQSTS_RXIRQ_MASK) {
        DEBUG("[kw41zrf] finished RXSEQ\n");
        handled_irqs |= ZLL_IRQSTS_RXIRQ_MASK;
        netdev->event_callback(netdev, NETDEV_EVENT_RX_COMPLETE);
        if (ZLL->PHY_CTRL & ZLL_PHY_CTRL_AUTOACK_MASK) {
            DEBUG("[kw41zrf]: perform TX ACK\n");
        }
    }

    if (irqsts & ZLL_IRQSTS_TXIRQ_MASK) {
        DEBUG("[kw41zrf] finished (ACK) TXSEQ\n");
        handled_irqs |= ZLL_IRQSTS_TXIRQ_MASK;
    }

    if (irqsts & ZLL_IRQSTS_SEQIRQ_MASK) {
        DEBUG("[kw41zrf] SEQIRQ (R)\n");
        handled_irqs |= ZLL_IRQSTS_SEQIRQ_MASK;
        kw41zrf_set_idle_sequence(dev);
    }

    kw41zrf_clear_irq_flags(handled_irqs);
}

static void _isr_event_seq_t(netdev_t *netdev)
{
    kw41zrf_t *dev = (kw41zrf_t *)netdev;
    uint32_t irqsts = ZLL->IRQSTS;
    uint32_t handled_irqs = 0;

    if (irqsts & ZLL_IRQSTS_TXIRQ_MASK) {
        DEBUG("[kw41zrf] finished TXSEQ (T)\n");
        handled_irqs |= ZLL_IRQSTS_TXIRQ_MASK;
    }

    if (irqsts & ZLL_IRQSTS_SEQIRQ_MASK) {
        DEBUG("[kw41zrf] SEQIRQ (T)\n");
        handled_irqs |= ZLL_IRQSTS_SEQIRQ_MASK;

        if (irqsts & ZLL_IRQSTS_CCAIRQ_MASK) {
            handled_irqs |= ZLL_IRQSTS_CCAIRQ_MASK;
            if (irqsts & ZLL_IRQSTS_CCA_MASK) {
                DEBUG("[kw41zrf] CCA CH busy\n");
                netdev->event_callback(netdev, NETDEV_EVENT_TX_MEDIUM_BUSY);
            }
            else {
                netdev->event_callback(netdev, NETDEV_EVENT_TX_COMPLETE);
            }
        }

        assert(dev->pending_tx != 0);
        dev->pending_tx--;
        kw41zrf_set_idle_sequence(dev);
    }

    kw41zrf_clear_irq_flags(handled_irqs);
}

/* Standalone CCA */
static void _isr_event_seq_cca(netdev_t *netdev)
{
    kw41zrf_t *dev = (kw41zrf_t *)netdev;
    uint32_t irqsts = ZLL->IRQSTS;
    uint32_t handled_irqs = 0;

    if ((irqsts & ZLL_IRQSTS_SEQIRQ_MASK) &&
        (irqsts & ZLL_IRQSTS_CCAIRQ_MASK)) {
        handled_irqs |= ZLL_IRQSTS_CCAIRQ_MASK | ZLL_IRQSTS_SEQIRQ_MASK;
        if (irqsts & ZLL_IRQSTS_CCA_MASK) {
            DEBUG("[kw41zrf] SEQIRQ (CCA), CCA CH busy\n");
        }
        else {
            DEBUG("[kw41zrf] SEQIRQ (CCA), CCA CH idle\n");
        }
        kw41zrf_set_idle_sequence(dev);
    }

    kw41zrf_clear_irq_flags(handled_irqs);
}

static void _isr_event_seq_tr(netdev_t *netdev)
{
    kw41zrf_t *dev = (kw41zrf_t *)netdev;
    uint32_t irqsts = ZLL->IRQSTS;
    uint32_t handled_irqs = 0;

    if (irqsts & ZLL_IRQSTS_TXIRQ_MASK) {
        DEBUG("[kw41zrf] finished TXSEQ (TR)\n");
        handled_irqs |= ZLL_IRQSTS_TXIRQ_MASK;
        if (ZLL->PHY_CTRL & ZLL_PHY_CTRL_RXACKRQD_MASK) {
            DEBUG("[kw41zrf] wait for RX ACK\n");
            kw41zrf_seq_timeout_on(dev, _MACACKWAITDURATION);
        }
    }

    if (irqsts & ZLL_IRQSTS_RXWTRMRKIRQ_MASK) {
        DEBUG("[kw41zrf] RXWTRMRKIRQ (TR)\n");
        handled_irqs |= ZLL_IRQSTS_RXWTRMRKIRQ_MASK;
    }

    if (irqsts & ZLL_IRQSTS_FILTERFAIL_IRQ_MASK) {
        DEBUG("[kw41zrf] FILTERFAILIRQ (TR)\n");
        handled_irqs |= ZLL_IRQSTS_FILTERFAIL_IRQ_MASK;
    }

    if (irqsts & ZLL_IRQSTS_RXIRQ_MASK) {
        DEBUG("[kw41zrf] got RX ACK\n");
        handled_irqs |= ZLL_IRQSTS_RXIRQ_MASK;
    }

    if (irqsts & ZLL_IRQSTS_SEQIRQ_MASK) {
        DEBUG("[kw41zrf] SEQIRQ (TR)\n");
        if (irqsts & ZLL_IRQSTS_CCAIRQ_MASK) {
            handled_irqs |= ZLL_IRQSTS_CCAIRQ_MASK;
            if (irqsts & ZLL_IRQSTS_CCA_MASK) {
                DEBUG("[kw41zrf] CCA CH busy\n");
                netdev->event_callback(netdev, NETDEV_EVENT_TX_MEDIUM_BUSY);
            }
        }

        handled_irqs |= ZLL_IRQSTS_SEQIRQ_MASK;
        assert(dev->pending_tx != 0);
        dev->pending_tx--;
        netdev->event_callback(netdev, NETDEV_EVENT_TX_COMPLETE);
        kw41zrf_seq_timeout_off(dev);
        kw41zrf_set_idle_sequence(dev);
    }
    else if (irqsts & ZLL_IRQSTS_TMR3IRQ_MASK) {
        DEBUG("[kw41zrf] RX timeout\n");
        assert(dev->pending_tx != 0);
        dev->pending_tx--;
        netdev->event_callback(netdev, NETDEV_EVENT_TX_NOACK);
        kw41zrf_seq_timeout_off(dev);
        kw41zrf_set_sequence(dev, dev->idle_state);
    }

    kw41zrf_clear_irq_flags(handled_irqs);
}

static void _isr_event_seq_ccca(netdev_t *netdev)
{
    kw41zrf_t *dev = (kw41zrf_t *)netdev;
    uint32_t irqsts = ZLL->IRQSTS;
    uint32_t handled_irqs = 0;

    if ((irqsts & ZLL_IRQSTS_CCAIRQ_MASK) &&
        (irqsts & ZLL_IRQSTS_SEQIRQ_MASK)) {
        handled_irqs |= ZLL_IRQSTS_CCAIRQ_MASK | ZLL_IRQSTS_SEQIRQ_MASK;
        DEBUG("[kw41zrf] CCCA CH idle\n");
        kw41zrf_seq_timeout_off(dev);
        kw41zrf_set_sequence(dev, dev->idle_state);
    }
    else if (irqsts & ZLL_IRQSTS_TMR3IRQ_MASK) {
        handled_irqs |= ZLL_IRQSTS_CCAIRQ_MASK | ZLL_IRQSTS_SEQIRQ_MASK;
        DEBUG("[kw41zrf] CCCA timeout\n");
        kw41zrf_seq_timeout_off(dev);
        kw41zrf_set_sequence(dev, dev->idle_state);
    }

    kw41zrf_clear_irq_flags(handled_irqs);
}

static void kw41zrf_netdev_isr(netdev_t *netdev)
{
    kw41zrf_mask_irqs();

    DEBUG("[kw41zrf] CTRL %08" PRIx32 ", IRQSTS %08" PRIx32 "\n",
          ZLL->PHY_CTRL, ZLL->IRQSTS);

    switch ((ZLL->PHY_CTRL & ZLL_PHY_CTRL_XCVSEQ_MASK) >> ZLL_PHY_CTRL_XCVSEQ_SHIFT) {
        case XCVSEQ_RECEIVE:
            _isr_event_seq_r(netdev);
            break;

        case XCVSEQ_TRANSMIT:
            _isr_event_seq_t(netdev);
            break;

        case XCVSEQ_CCA:
            _isr_event_seq_cca(netdev);
            break;

        case XCVSEQ_TX_RX:
            _isr_event_seq_tr(netdev);
            break;

        case XCVSEQ_CONTINUOUS_CCA:
            _isr_event_seq_ccca(netdev);
            break;

        case XCVSEQ_IDLE:
            DEBUG("[kw41zrf] IRQ while IDLE\n");
            break;

        default:
            DEBUG("[kw41zrf] undefined seq state in isr\n");
            break;
    }

    uint32_t irqsts = ZLL->IRQSTS;
    uint32_t handled_irqs = 0;
    if (irqsts & ZLL_IRQSTS_PLL_UNLOCK_IRQ_MASK) {
        DEBUG("[kw41zrf] untreated PLL_UNLOCK_IRQ\n");
        handled_irqs |= ZLL_IRQSTS_PLL_UNLOCK_IRQ_MASK;
    }
    if (irqsts & ZLL_IRQSTS_WAKE_IRQ_MASK) {
        DEBUG("[kw41zrf] untreated WAKE_IRQ\n");
        handled_irqs |= ZLL_IRQSTS_WAKE_IRQ_MASK;
    }
    kw41zrf_clear_irq_flags(handled_irqs);

    if (ENABLE_DEBUG) {
        /* for debugging only */
        if (ZLL->IRQSTS & 0x000f017f) {
            DEBUG("[kw41zrf] IRQSTS contains untreated IRQs: 0x%08"PRIx32"\n",
                  (ZLL->IRQSTS & 0x000f017f));
        }
    }

    kw41zrf_unmask_irqs();
}

const netdev_driver_t kw41zrf_driver = {
    .init = kw41zrf_netdev_init,
    .send = kw41zrf_netdev_send,
    .recv = kw41zrf_netdev_recv,
    .get  = kw41zrf_netdev_get,
    .set  = kw41zrf_netdev_set,
    .isr  = kw41zrf_netdev_isr,
};

/** @} */
