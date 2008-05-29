/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

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

        muxer mux;
        setfdeventmask(mux, sfd, AUG_FDEVENTRD);

        for (;;) {
            while (AUG_FAILINTR == waitfdevents(mux))
                ;

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
