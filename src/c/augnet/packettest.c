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
#include "augnet.h"
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>

static const char DATA[] = "stale event://www.xofy.org/aug";

int
main(int argc, char* argv[])
{
    struct aug_packet pkt;
    char buf[AUG_PACKETSIZE];

    if (!aug_autotlx())
        return 1;

    aug_setpacket("augd", 101, 202, 303, DATA, sizeof(DATA), &pkt);

    aug_encodepacket(&pkt, buf);
    memset(&pkt, 0, sizeof(pkt));
    aug_decodepacket(buf, &pkt);

    /*
    unsigned sess_;
    unsigned short type_;
    aug_seqno_t seqno_;
    unsigned size_;
    char data_[AUG_PKTDATASIZE];
     */

    if (1 != pkt.proto_) {
        fprintf(stderr, "unexpected protocol [%d]\n", (int)pkt.proto_);
        return 1;
    }

    if (0 != strcmp(pkt.node_, "augd")) {
        fprintf(stderr, "unexpected node name [%s]\n", pkt.node_);
        return 1;
    }

    if (101 != pkt.sess_) {
        fprintf(stderr, "unexpected time [%u]\n", (unsigned)pkt.sess_);
        return 1;
    }

    if (202 != pkt.type_) {
        fprintf(stderr, "unexpected type [%u]\n", (unsigned)pkt.type_);
        return 1;
    }

    if (303 != pkt.seqno_) {
        fprintf(stderr, "unexpected sequence [%d]\n", (int)pkt.seqno_);
        return 1;
    }

    if (sizeof(DATA) != pkt.size_) {
        fprintf(stderr, "unexpected size [%d]\n", (int)pkt.size_);
        return 1;
    }

    if (0 != strcmp(pkt.data_, DATA)) {
        fprintf(stderr, "unexpected data [%s]\n", pkt.data_);
        return 1;
    }

    return 0;
}
