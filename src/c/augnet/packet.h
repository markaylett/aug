/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGNET_PACKET_H
#define AUGNET_PACKET_H

/**
 * @file augnet/packet.h
 *
 * Network event packet format.
 */

#include "augnet/config.h"

#include "augsys/socket.h" /* AUG_MAXHOSTSERVLEN */

/**
 * @defgroup Packet Packet
 */

/**
 * @defgroup PacketLimits Packet Limits
 *
 * @ingroup Packet
 *
 * @{
 */

#define AUG_PKTMAGICSIZE  4
#define AUG_PKTSEQNOSIZE  sizeof(uint32_t)
#define AUG_PKTVERNOSIZE  sizeof(uint16_t)
#define AUG_PKTTYPESIZE   sizeof(uint16_t)
#define AUG_PKTADDRLEN    AUG_MAXHOSTSERVLEN
#define AUG_PKTBODYSIZE   376
#define AUG_PKTMETHODLEN  64
#define AUG_PKTURILEN    (AUG_PKTBODYSIZE - AUG_PKTMETHODLEN)

/** @} */

/**
 * @defgroup PacketOffsets Packet Offsets
 *
 * @ingroup Packet
 *
 * @{
 */

#define AUG_PKTMAGICOFF   0
#define AUG_PKTSEQNOOFF  (AUG_PKTMAGICOFF + AUG_PKTMAGICSIZE)
#define AUG_PKTVERNOOFF  (AUG_PKTSEQNOOFF + AUG_PKTSEQNOSIZE)
#define AUG_PKTTYPEOFF   (AUG_PKTVERNOOFF + AUG_PKTVERNOSIZE)
#define AUG_PKTADDROFF   (AUG_PKTTYPEOFF + AUG_PKTTYPESIZE)
#define AUG_PKTBODYOFF   (AUG_PKTADDROFF + AUG_PKTADDRLEN)
#define AUG_PKTMETHODOFF  AUG_PKTBODYOFF
#define AUG_PKTURIOFF    (AUG_PKTMETHODOFF + AUG_PKTMETHODLEN)

/** @} */

#define AUG_PKTOPEN  1
#define AUG_PKTCLOSE 2
#define AUG_PKTHBEAT 3
#define AUG_PKTLOST  4
#define AUG_PKTEVENT 5

/**
 * Packet structure.
 */

struct aug_packet {
    unsigned seqno_, verno_, type_;
    char addr_[AUG_PKTADDRLEN + 1];
    union {
        struct {
            char method_[AUG_PKTMETHODLEN + 1];
            char uri_[AUG_PKTURILEN + 1];
        } event_;
        char ext_[AUG_PKTBODYSIZE];
    } content_;
};

/**
 * Verify that packet elements do not exceed limits.
 *
 * @param packet Packet object.
 */

AUGNET_API aug_result
aug_verifypacket(const struct aug_packet* packet);

/**
 * Encode packet into network buffer.
 *
 * @param buf Output buffer.  Must be at least #AUG_PACKETSIZE in length.
 *
 * @param packet Input packet object.
 *
 * @return @a buf, or null on error.
 */

AUGNET_API char*
aug_encodepacket(char* buf, const struct aug_packet* pkt);

/**
 * Decode network buffer into packet.
 *
 * @param packet Output packet.
 *
 * @param buf Input network buffer.
 *
 * @return @a pkt, or null on error.
 */

AUGNET_API struct aug_packet*
aug_decodepacket(struct aug_packet* pkt, const char* buf);

/**
 * Decode network buffer's sequence number only.
 *
 * @param seqno Output sequence number.
 *
 * @param buf Input network buffer.
 *
 * @return @a seqno, or null on error.
 */

AUGNET_API unsigned*
aug_decodeseqno(unsigned* seqno, const char* buf);

#endif /* AUGNET_PACKET_H */
