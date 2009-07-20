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

#include <string.h>

#define MAGIC_ "aug"

static void
packstring_(char* dst, const char* src, size_t size)
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

AUGNET_API aug_result
aug_verifypacket(const struct aug_packet* packet)
{
    if ('\0' == packet->chan_[0]) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ENULL,
                       AUG_MSG("empty channel name"));
        return AUG_FAILERROR;
    }

    return AUG_SUCCESS;
}

AUGNET_API char*
aug_encodepacket(const struct aug_packet* pkt, char* buf)
{
    memcpy(buf + AUG_PKTMAGICOFF, MAGIC_, AUG_PKTMAGICSIZE);

    aug_encode16(buf + AUG_PKTPROTOOFF, pkt->proto_);
    packstring_(buf + AUG_PKTCHANOFF, pkt->chan_, AUG_PKTCHANLEN);
    aug_encode32(buf + AUG_PKTSEQNOOFF, pkt->seqno_);
    aug_encode64(buf + AUG_PKTTIMEOFF, pkt->time_);
    aug_encode16(buf + AUG_PKTFLAGSOFF, pkt->flags_);
    aug_encode16(buf + AUG_PKTTYPEOFF, pkt->type_);

    memcpy(buf + AUG_PKTDATAOFF, pkt->data_, sizeof(pkt->data_));
    return buf;
}

AUGNET_API struct aug_packet*
aug_decodepacket(const char* buf, struct aug_packet* pkt)
{
    if (0 != memcmp(buf + AUG_PKTMAGICOFF, MAGIC_, AUG_PKTMAGICSIZE)) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid packet header"));
        return NULL;
    }

    pkt->proto_ = aug_decode16(buf + AUG_PKTPROTOOFF);
    unpackstring_(pkt->chan_, buf + AUG_PKTCHANOFF, AUG_PKTCHANLEN);
    pkt->seqno_ = aug_decode32(buf + AUG_PKTSEQNOOFF);
    pkt->time_ = aug_decode64(buf + AUG_PKTTIMEOFF);
    pkt->flags_ = aug_decode16(buf + AUG_PKTFLAGSOFF);
    pkt->type_ = aug_decode16(buf + AUG_PKTTYPEOFF);

    memcpy(pkt->data_, buf + AUG_PKTDATAOFF, sizeof(pkt->data_));
    return pkt;
}
