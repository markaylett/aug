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

        aug_errinfo errinfo;
        scoped_init init(errinfo);

        try {

            if (argc < 3) {
                aug_error("usage: mcastrecv <mcast> <serv> [ifname]");
                return 1;
            }

            inetaddr in(argv[1]);
            smartfd sfd(aug::socket(family(in), SOCK_DGRAM));
            setreuseaddr(sfd, true);

            endpoint ep(inetany(family(in)), htons(atoi(argv[2])));
            aug::bind(sfd, ep);

            joinmcast(sfd, in, 4 == argc ? argv[3] : 0);

            muxer mux;
            setfdeventmask(mux, sfd, AUG_FDEVENTRD);

            for (;;) {
                while (AUG_RETINTR == waitfdevents(mux))
                    ;

                char buf[AUG_NETEVENT_SIZE];
                size_t size(read(sfd, buf, sizeof(buf)));

                aug_netevent event;
                aug_unpacknetevent(&event, buf);

                aug_info("recv: name=[%s], seq=[%d]", event.name_,
                         event.seq_);
            }

        } catch (const errinfo_error& e) {
            perrinfo(e, "aug::errorinfo_error");
        } catch (const exception& e) {
            aug_error("std::exception: %s", e.what());
        }
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
