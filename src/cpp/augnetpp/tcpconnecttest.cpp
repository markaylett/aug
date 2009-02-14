/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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

typedef logic_error error;

int
main(int argc, char* argv[])
{
    try {
        autotlx();

        mpoolptr mp(getmpool(aug_tlx));
        endpoint ep(null);
        tcpconnect conn(mp, "127.0.0.1", "10000");

        bool est(false);
        autosd sd(tryconnect(conn, ep, est));
        if (!est) {

            muxer mux(mp);
            setmdeventmask(mux, sd, AUG_MDEVENTCONN);
            waitmdevents(mux);

            // Assuming that there is no endpoint, an exception should now be
            // thrown.

            try {
                sd = tryconnect(conn, ep, est);
            } catch (...) {
                if (ECONNREFUSED == aug_errno(aug_tlerr))
                    return 0;
                throw;
            }
        }
        cerr << "error not thrown by tryconnect()\n";

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
