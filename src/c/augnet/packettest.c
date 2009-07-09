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

    if (!aug_autotlx())
        return 1;

    pkt.ver_ = 101;
    pkt.type_ = AUG_PKTEVENT;
    pkt.seq_ = 202;

    strcpy(pkt.addr_, "127.0.0.1:1972");
    strcpy(pkt.content_.event_.method_, "stale");
    strcpy(pkt.content_.event_.uri_, "event://www.xofy.org/aug");

    aug_encodepacket(buf, &pkt);
    memset(&pkt, 0, sizeof(pkt));
    aug_decodepacket(&pkt, buf);

    if (101 != pkt.ver_) {
        fprintf(stderr, "unexpected version [%d]\n", pkt.ver_);
        return 1;
    }

    if (AUG_PKTEVENT != pkt.type_) {
        fprintf(stderr, "unexpected type [%d]\n", pkt.type_);
        return 1;
    }

    if (202 != pkt.seq_) {
        fprintf(stderr, "unexpected seq [%d]\n", pkt.seq_);
        return 1;
    }

    if (0 != strcmp(pkt.addr_, "127.0.0.1:1972")) {
        fprintf(stderr, "unexpected addr [%s]\n", pkt.addr_);
        return 1;
    }

    if (0 != strcmp(pkt.content_.event_.method_, "stale")) {
        fprintf(stderr, "unexpected method [%s]\n",
                pkt.content_.event_.method_);
        return 1;
    }

    if (0 != strcmp(pkt.content_.event_.uri_, "event://www.xofy.org/aug")) {
        fprintf(stderr, "unexpected uri [%s]\n", pkt.content_.event_.uri_);
        return 1;
    }

    return 0;
}
