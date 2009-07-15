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

int
main(int argc, char* argv[])
{
    struct aug_packet pkt;
    char buf[AUG_PACKETSIZE];
    aug_time now = time(0) * 1000;

    if (!aug_autotlx())
        return 1;

    pkt.proto_ = 1;
    strcpy(pkt.chan_, "127.0.0.1:1972");
    pkt.seqno_ = 101;
    pkt.verno_ = 202;
    pkt.time_ = now;
    pkt.flags_ = 0;
    pkt.type_ = 2;
    strcpy(pkt.data_, "stale event://www.xofy.org/aug");

    aug_encodepacket(&pkt, buf);
    memset(&pkt, 0, sizeof(pkt));
    aug_decodepacket(buf, &pkt);

    if (1 != pkt.proto_) {
        fprintf(stderr, "unexpected protocol [%d]\n", (int)pkt.proto_);
        return 1;
    }

    if (0 != strcmp(pkt.chan_, "127.0.0.1:1972")) {
        fprintf(stderr, "unexpected channel name [%s]\n", pkt.chan_);
        return 1;
    }

    if (101 != pkt.seqno_) {
        fprintf(stderr, "unexpected sequence [%d]\n", (int)pkt.seqno_);
        return 1;
    }

    if (202 != pkt.verno_) {
        fprintf(stderr, "unexpected version [%d]\n", (int)pkt.verno_);
        return 1;
    }

    if (now != pkt.time_) {
        fprintf(stderr, "unexpected time [%u]\n", (unsigned)pkt.time_);
        return 1;
    }

    if (0 != pkt.flags_) {
        fprintf(stderr, "unexpected flags [%d]\n", (int)pkt.flags_);
        return 1;
    }

    if (2 != pkt.type_) {
        fprintf(stderr, "unexpected type [%d]\n", (int)pkt.type_);
        return 1;
    }

    if (0 != strcmp(pkt.data_, "stale event://www.xofy.org/aug")) {
        fprintf(stderr, "unexpected data [%s]\n", pkt.data_);
        return 1;
    }

    return 0;
}
