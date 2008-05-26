/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnetpp/tcpconnect.hpp"

#include "augsyspp/base.hpp"
#include "augsyspp/endpoint.hpp"
#include "augsyspp/muxer.hpp"
#include "augsyspp/unistd.hpp"

#include "augctx/errno.h"
#include "augctx/log.h"

using namespace aug;
using namespace std;

typedef logic_error error;

int
main(int argc, char* argv[])
{
    try {
        atbasixtlx();

        endpoint ep(null);
        tcpconnect conn("127.0.0.1", "10000");

        bool est(false);
        autosd sd(tryconnect(conn, ep, est));
        if (!est) {

            muxer mux;
            setfdeventmask(mux, sd, AUG_FDEVENTCONN);
            waitfdevents(mux);

            // Assuming that there is no endpoint, an exception should now be
            // thrown.

            try {
                sd = tryconnect(conn, ep, est);
            } catch (...) {
                if (ECONNREFUSED == aug_errno())
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
