/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnetpp/connector.hpp"

#include "augsyspp/base.hpp"
#include "augsyspp/endpoint.hpp"
#include "augsyspp/mplexer.hpp"
#include "augsyspp/unistd.hpp"

#include "augsys/log.h"

using namespace aug;
using namespace std;

typedef logic_error error;

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    aug_atexitinit(&errinfo);

    try {

        endpoint ep(null);
        connector ctor("127.0.0.1", "10000");

        std::pair<smartfd, bool> xy(tryconnect(ctor, ep));
        if (!xy.second) {

            mplexer mp;
            setfdeventmask(mp, xy.first, AUG_FDEVENTALL);
            waitfdevents(mp);

            // Assuming that there is no endpoint, an exception should now be
            // thrown.

            try {
                xy = tryconnect(ctor, ep);
            } catch (...) {
                if (ECONNREFUSED == aug_errno())
                    return 0;
                throw;
            }
        }
        throw error("error not thrown by tryconnect()");

    } AUG_PERRINFOCATCH;
    return 1;
}
