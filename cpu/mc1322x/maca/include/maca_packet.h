/*
 * maca_packet.h - defines a rxtx packet for the MACA driver
 * Copyright (C) 2013 Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 *
 * This source code is licensed under the GNU General Public License,
 * Version 3.  See the file LICENSE for more details.
 *
 * This file is part of RIOT.
 */

#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

/* does not include 2 byte FCS checksum */
#ifndef MACA_MAX_PAYLOAD_SIZE
#define MACA_MAX_PAYLOAD_SIZE 125
#endif

#define PACKET_STATS 0

struct packet {
    uint8_t length;                 /*< does not include FCS checksum */
    volatile struct packet *left;
    volatile struct packet *right;
    /* offset into data for first byte of the packet payload */
    /* On TX this should be 0 */
    /* On RX this should be 1 since the maca puts the length as the first byte*/
    uint8_t offset;
    uint8_t lqi;
    uint8_t status;
    uint32_t rx_time;
#if PACKET_STATS
    uint8_t seen;
    uint8_t post_tx;
    uint8_t get_free;
    uint8_t rxd;
#endif
    /* +2 for FCS; + 1 since maca returns the length as the first byte */
    uint8_t data[MACA_MAX_PAYLOAD_SIZE + 2 + 1]; 
};
typedef struct packet maca_packet_t;

#endif