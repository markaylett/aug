/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrvpp.hpp"
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

        autobasictlx();

        if (argc < 3) {
            aug_ctxerror(aug_tlx, "usage: mcastrecv <mcast> <serv> [ifname]");
            return 1;
        }

        inetaddr in(argv[1]);
        autosd sfd(aug::socket(family(in), SOCK_DGRAM));
        setreuseaddr(sfd, true);

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

            char buf[AUG_NETEVENT_SIZE];
            size_t size(read(sfd, buf, sizeof(buf)));

            aug_netevent event;
            aug_unpacknetevent(&event, buf);

            aug_ctxinfo(aug_tlx, "recv: name=[%s], seq=[%d]", event.name_,
                        event.seq_);
        }
        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
