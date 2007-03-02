/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsyspp/mplexer.hpp"
#include "augsyspp/socket.hpp"

#include "augsys/log.h"

#include <vector>

using namespace aug;
using namespace std;

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    aug_atexitinit(&errinfo);

    try {
        vector<smartfd> v;
        mplexer mp;
        for (int i(0); i < 256; ++i) {
            pair<smartfd, smartfd> xy(socketpair(AF_UNIX, SOCK_STREAM, 0));
            try {
                setioeventmask(mp, xy.first, AUG_IOEVENTRD);
                setioeventmask(mp, xy.second, AUG_IOEVENTRD);
                v.push_back(xy.first);
                v.push_back(xy.second);
            } catch (const system_error& e) {

                // When the mplexer is implemented in terms of select(), the
                // FD_SETSIZE limit may be reached.  In which case, continue
                // to loop.

                if (EMFILE != aug_errno())
                    throw;
            }
        }
        return 0;
    } AUG_PERRINFOCATCH;
    return 1;
}