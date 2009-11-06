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
 *   4: magic
 *   2: proto
 * 128: node
 *   4: sess
 *   2: type
 *   8: seqno
 *   2: size
 * 366: buf
 *
 * @{
 */

#define AUG_PKTMAGICSIZE 4
#define AUG_PKTPROTOSIZE sizeof(uint16_t)
#define AUG_PKTNODELEN   AUG_MAXADDRLEN
#define AUG_PKTSESSSIZE  sizeof(uint32_t)
#define AUG_PKTTYPESIZE  sizeof(uint16_t)
#define AUG_PKTSEQNOSIZE sizeof(uint64_t)
#define AUG_PKTSIZESIZE  sizeof(uint16_t)
#define AUG_PKTDATASIZE  366

/** @} */

/**
 * @defgroup PacketOffsets Packet Offsets
 *
 * @ingroup Packet
 *
 * @{
 */

#define AUG_PKTMAGICOFF  0
#define AUG_PKTPROTOOFF  (AUG_PKTMAGICOFF + AUG_PKTMAGICSIZE)
#define AUG_PKTNODEOFF   (AUG_PKTPROTOOFF + AUG_PKTPROTOSIZE)
#define AUG_PKTSESSOFF   (AUG_PKTNODEOFF + AUG_PKTNODELEN)
#define AUG_PKTTYPEOFF   (AUG_PKTSESSOFF + AUG_PKTSESSSIZE)
#define AUG_PKTSEQNOOFF  (AUG_PKTTYPEOFF + AUG_PKTTYPESIZE)
#define AUG_PKTSIZEOFF   (AUG_PKTSEQNOOFF + AUG_PKTSEQNOSIZE)
#define AUG_PKTDATAOFF   (AUG_PKTSIZEOFF + AUG_PKTSIZESIZE)

/** @} */

/**
 * @defgroup PacketTypes Packet Types
 *
 * @ingroup Packet
 *
 * @{
 */

#define AUG_PKTUP     1
#define AUG_PKTDOWN   2
#define AUG_PKTGAP    3

#define AUG_PKTBASE  16
#define AUG_PKTHBEAT (AUG_PKTBASE + 0)
#define AUG_PKTUSER  (AUG_PKTBASE + 1)

/** @} */

typedef unsigned long aug_seqno_t;

/**
 * Packet structure.
 */

struct aug_packet {
    unsigned proto_;
    char node_[AUG_PKTNODELEN + 1];
    unsigned sess_;
    unsigned short type_;
    aug_seqno_t seqno_;
    unsigned size_;
    char data_[AUG_PKTDATASIZE];
};

AUGNET_API struct aug_packet*
aug_setpacket(const char* node, unsigned sess, unsigned short type,
              aug_seqno_t seqno, const void* data, unsigned size,
              struct aug_packet* pkt);

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
 * @param packet Input packet object.
 *
 * @param buf Output buffer.  Must be at least #AUG_PACKETSIZE in length.
 *
 * @return @a buf, or null on error.
 */

AUGNET_API char*
aug_encodepacket(const struct aug_packet* pkt, char* buf);

/**
 * Decode network buffer into packet.
 *
 * @param buf Input network buffer.
 *
 * @param packet Output packet.
 *
 * @return @a pkt, or null on error.
 */

AUGNET_API struct aug_packet*
aug_decodepacket(const char* buf, struct aug_packet* pkt);

#endif /* AUGNET_PACKET_H */
