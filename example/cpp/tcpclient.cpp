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
#include "augnetpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

int
main(int argc, char* argv[])
{
    static const char MSG[] = "try to exhaust tcp window\r\n";

    try {

        autotlx();

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
