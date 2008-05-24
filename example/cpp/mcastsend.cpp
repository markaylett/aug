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

static unsigned seq_ = 0;

static char*
heartbeat_(char* buf)
{
    aug_netevent event;
    event.proto_ = 1;
    strcpy(event.name_, "test");
    event.state_ = 1;
    event.seq_ = ++seq_;
    event.hbsec_ = 1;
    event.weight_ = 1;
    event.type_ = 1;
    aug_packnetevent(buf, &event);
    return buf;
}

int
main(int argc, char* argv[])
{
    try {

        start();

        if (argc < 3) {
            aug_ctxerror(aug_tlx, "usage: mcastsend <mcast> <serv> [ifname]");
            return 1;
        }

        inetaddr in(argv[1]);
        autosd sfd(aug::socket(family(in), SOCK_DGRAM));
        if (4 == argc)
            setmcastif(sfd, argv[3]);

        endpoint ep(in, htons(atoi(argv[2])));

        char event[AUG_NETEVENT_SIZE];
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
