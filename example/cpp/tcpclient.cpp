/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnetpp.hpp"
#include "augsyspp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

int
main(int argc, char* argv[])
{
    static const char MSG[] = "try to exhaust tcp window\n";

    struct aug_errinfo errinfo;
    aug_atexitinit(&errinfo);

    try {

        if (argc < 3) {
            aug_error("usage: tcpclient <host> <serv>");
            return 1;
        }

        // Simulate rude client behaviour: continuously send, without
        // receiving - try to exhaust tcp window and break server.

        endpoint ep(null);
        smartfd sfd(tcpconnect(argv[1], argv[2], ep));
        for (int i(0); i < 1000000; ++i) {
            send(sfd, MSG, sizeof(MSG) - 1, 0);
            if (0 == i % 1000 && 0 < i)
                aug_info("%d", i);
        }

    } AUG_PERRINFOCATCH;
    return 1;
}
