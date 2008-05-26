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
    static const char MSG[] = "try to exhaust tcp window\r\n";

    try {

        atbasixtlx();

        if (argc < 3) {
            aug_ctxerror(aug_tlx, "usage: tcpclient <host> <serv>");
            return 1;
        }

        // Simulate rude client behaviour: fast send and slow receive to
        // echo-like server - try to exhaust tcp window and break server.

        endpoint ep(null);
        autosd sfd(tcpclient(argv[1], argv[2], ep));
        for (int i(0); i < 1000000; ++i) {
            send(sfd, MSG, sizeof(MSG) - 1, 0);
            if (0 == i % 3 && 0 < i) {
                char buf[sizeof(MSG) - 1];
                recv(sfd, buf, sizeof(buf), 0);
            }
            if (0 == i % 10 && 0 < i) {
                char buf[(sizeof(MSG) - 1) *  2];
                recv(sfd, buf, sizeof(buf), 0);
            }
            if (0 == i % 1000 && 0 < i)
                aug_ctxinfo(aug_tlx, "%d", i);
        }

        shutdown(sfd, SHUT_WR);
        for (;;) {
            char buf[sizeof(MSG) - 1];
            if (0 == recv(sfd, buf, sizeof(buf), 0))
                break;
        }

        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
