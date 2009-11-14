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
#include "augservpp.hpp"
#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

static unsigned inst_;
static unsigned seqno_ = 0;

static char*
heartbeat_(char* buf)
{
    aug_packet pkt;
    aug_setpacket("mcastsend", inst_, AUG_PKTHBEAT, ++seqno_, 0, 0, &pkt);
    return aug_encodepacket(&pkt, buf);
}

int
main(int argc, char* argv[])
{
    try {

        autotlx();

        if (argc < 3) {
            aug_ctxerror(aug_tlx, "usage: mcastsend <mcast> <serv> [ifname]");
            return 1;
        }

        inst_ = getpid();

        inetaddr in(argv[1]);
        autosd sfd(aug::socket(family(in), SOCK_DGRAM));
        if (4 == argc)
            setmcastif(sfd, argv[3]);

        endpoint ep(in, htons(atoi(argv[2])));

        char event[AUG_PACKETSIZE];
        for (int i(0); i < 3; ++i) {
            heartbeat_(event);
            sendto(sfd, event, sizeof(event), 0, ep);
            aug_msleep(1000);
        }

        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
