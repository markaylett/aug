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
    if (UINT16_MAX < packet->ver_) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ELIMIT,
                       AUG_MSG("maximum packet version exceeded"));
        return AUG_FAILERROR;
    }

    if (UINT16_MAX < packet->type_) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ELIMIT,
                       AUG_MSG("maximum packet type exceeded"));
        return AUG_FAILERROR;
    }

    if (UINT32_MAX < packet->seq_) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ELIMIT,
                       AUG_MSG("maximum packet sequence exceeded"));
        return AUG_FAILERROR;
    }

    if ('\0' == packet->addr_[0]) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ENULL,
                       AUG_MSG("null packet address"));
        return AUG_FAILERROR;
    }

    return AUG_SUCCESS;
}

AUGNET_API char*
aug_encodepacket(char* buf, const struct aug_packet* pkt)
{
    memcpy(buf + AUG_PKTMAGICOFF, MAGIC_, AUG_PKTMAGICSIZE);

    /* Standard header fields. */

    aug_encode16(buf + AUG_PKTVEROFF, (uint16_t)pkt->ver_);
    aug_encode16(buf + AUG_PKTTYPEOFF, (uint16_t)pkt->type_);
    aug_encode32(buf + AUG_PKTSEQOFF, (uint32_t)pkt->seq_);
    packstring_(buf + AUG_PKTADDROFF, pkt->addr_, AUG_PKTADDRLEN);

    switch (pkt->type_) {
    case AUG_PKTOPEN:
    case AUG_PKTCLOSE:
    case AUG_PKTHBEAT:
    case AUG_PKTLOST:
        /* Zero-pad content. */
        memset(buf + AUG_PKTBODYOFF, 0, AUG_PKTBODYSIZE);
        break;
    case AUG_PKTEVENT:
        packstring_(buf + AUG_PKTMETHODOFF, pkt->content_.event_.method_,
                    AUG_PKTMETHODLEN);
        packstring_(buf + AUG_PKTURIOFF, pkt->content_.event_.uri_,
                    AUG_PKTURILEN);
        /* Packet full.  No need to zero-pad. */
        break;
    default:
        /* Use extension body. */
        memcpy(buf + AUG_PKTBODYOFF, pkt->content_.ext_,
               sizeof(pkt->content_.ext_));
        break;
    }
    return buf;
}

AUGNET_API struct aug_packet*
aug_decodepacket(struct aug_packet* pkt, const char* buf)
{
    if (0 != memcmp(buf + AUG_PKTMAGICOFF, MAGIC_, AUG_PKTMAGICSIZE)) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid packet header"));
        return NULL;
    }

    /* Standard header fields. */

    pkt->ver_ = aug_decode16(buf + AUG_PKTVEROFF);
    pkt->type_ = aug_decode16(buf + AUG_PKTTYPEOFF);
    pkt->seq_ = aug_decode32(buf + AUG_PKTSEQOFF);
    unpackstring_(pkt->addr_, buf + AUG_PKTADDROFF, AUG_PKTADDRLEN);

    switch (pkt->type_) {
    case AUG_PKTOPEN:
    case AUG_PKTCLOSE:
    case AUG_PKTHBEAT:
    case AUG_PKTLOST:
        /* Header only. */
        break;
    case AUG_PKTEVENT:
        unpackstring_(pkt->content_.event_.method_, buf + AUG_PKTMETHODOFF,
                      AUG_PKTMETHODLEN);
        unpackstring_(pkt->content_.event_.uri_, buf + AUG_PKTURIOFF,
                      AUG_PKTURILEN);
        break;
    default:
        /* Use extension body. */
        memcpy(pkt->content_.ext_, buf + AUG_PKTBODYOFF,
               sizeof(pkt->content_.ext_));
        break;
    }
    return pkt;
}
