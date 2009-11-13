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

int
main(int argc, char* argv[])
{
    try {

        autotlx();

        if (argc < 3) {
            aug_ctxerror(aug_tlx, "usage: mcastrecv <mcast> <serv> [ifname]");
            return 1;
        }

        inetaddr in(argv[1]);
        autosd sfd(aug::socket(family(in), SOCK_DGRAM));

        // Reuse port.
        setreuseaddr(sfd, true);
        // Loopback.
        setmcastloop(sfd, true);

        endpoint ep(inetany(family(in)), htons(atoi(argv[2])));
        aug::bind(sfd, ep);

        joinmcast(sfd, in, 4 == argc ? argv[3] : 0);

        muxer mux(getmpool(aug_tlx));
        setmdeventmask(mux, sfd, AUG_MDEVENTRDEX);

        // FIXME: implementation assumes a level-triggered interface, which it
        // is not.

        for (;;) {

            try {
                waitmdevents(mux);
            } catch (const intr_exception&) {
                continue; // While interrupted.
            }

            char buf[AUG_PACKETSIZE];
            size_t size(read(sfd, buf, sizeof(buf)));

            aug_packet pkt;
            aug_decodepacket(buf, &pkt);

            aug_ctxinfo(aug_tlx, "recv: seqno=[%u], type=[%u]",
                        static_cast<unsigned>(pkt.seqno_),
                        static_cast<unsigned>(pkt.type_));
        }
        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
