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
#define AUGNET_BUILD
#include "augnet/packet.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/endian.h"
#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/string.h"

#include <string.h>

#define MAGIC_ "aug"

static void
packstring_(const char* src, char* dst, size_t size)
{
    size_t len = strlen(src);
    memcpy(dst, src, len);
    memset(dst + len, 0, size - len);
}

static void
unpackstring_(char* dst, const char* src, size_t size)
{
    memcpy(dst, src, size);
    dst[size] = '\0';
}

AUGNET_API struct aug_packet*
aug_setpacket(const char* node, unsigned inst, unsigned short type,
              aug_seqno_t seqno, const void* data, unsigned size,
              struct aug_packet* pkt)
{
    size = AUG_MIN(size, sizeof(pkt->data_));
    pkt->proto_ = 1;
    aug_strlcpy(pkt->node_, node, sizeof(pkt->node_));
    pkt->inst_ = inst;
    pkt->type_ = type;
    pkt->seqno_ = seqno;
    pkt->size_ = AUG_MIN(size, sizeof(pkt->data_));
    if (size)
        memcpy(pkt->data_, data, size);
    return pkt;
}

AUGNET_API aug_result
aug_verifypacket(const struct aug_packet* packet)
{
    if ('\0' == packet->node_[0]) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_ENULL,
                        AUG_MSG("empty node name"));
        return -1;
    }

    return 0;
}

AUGNET_API char*
aug_encodepacket(const struct aug_packet* pkt, char* buf)
{
    unsigned size;

    memcpy(buf + AUG_PKTMAGICOFF, MAGIC_, AUG_PKTMAGICSIZE);

    aug_encode16(pkt->proto_, buf + AUG_PKTPROTOOFF);
    packstring_(pkt->node_, buf + AUG_PKTNODEOFF, AUG_PKTNODELEN);
    aug_encode32(pkt->inst_, buf + AUG_PKTINSTOFF);
    aug_encode16(pkt->type_, buf + AUG_PKTTYPEOFF);
    aug_encode64(pkt->seqno_, buf + AUG_PKTSEQNOOFF);

    size = AUG_MIN(pkt->size_, sizeof(pkt->data_));
    aug_encode16(size, buf + AUG_PKTSIZEOFF);

    memcpy(buf + AUG_PKTDATAOFF, pkt->data_, size);
    /* Zero pad. */
    if (size < sizeof(pkt->data_))
        memset(buf + AUG_PKTDATAOFF + size, 0, sizeof(pkt->data_) - size);
    return buf;
}

AUGNET_API struct aug_packet*
aug_decodepacket(const char* buf, struct aug_packet* pkt)
{
    if (0 != memcmp(buf + AUG_PKTMAGICOFF, MAGIC_, AUG_PKTMAGICSIZE)) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid packet header"));
        return NULL;
    }

    pkt->proto_ = aug_decode16(buf + AUG_PKTPROTOOFF);
    unpackstring_(pkt->node_, buf + AUG_PKTNODEOFF, AUG_PKTNODELEN);
    pkt->inst_ = aug_decode32(buf + AUG_PKTINSTOFF);
    pkt->type_ = aug_decode16(buf + AUG_PKTTYPEOFF);
    pkt->seqno_ = (aug_seqno_t)aug_decode64(buf + AUG_PKTSEQNOOFF);

    /* Be defensive with data from wire. */

    pkt->size_ = AUG_MIN(aug_decode16(buf + AUG_PKTSIZEOFF),
                         sizeof(pkt->data_));
    memcpy(pkt->data_, buf + AUG_PKTDATAOFF, sizeof(pkt->data_));
    return pkt;
}
