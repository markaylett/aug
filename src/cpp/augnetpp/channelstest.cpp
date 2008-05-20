/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augnetpp.hpp"
#include "augsyspp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

namespace {

    pair<channelobptr, channelobptr>
    plainpair(mpoolref mpool, aug_muxer_t muxer)
    {
        autosds sds(socketpair(AF_UNIX, SOCK_STREAM, 0));
        return make_pair(createplain(mpool, aug_nextid(), sds[0], muxer),
                         createplain(mpool, aug_nextid(), sds[1], muxer));
    }

    unsigned rd_, wr_;

    bool
    cb(unsigned id, streamobref streamob,  unsigned short events)
    {
        cout << "ready: " << id << endl;
        return true;
    }
}

int
main(int argc, char* argv[])
{
    try {
        start();

        mpoolptr mp(getmpool(aug_tlx));
        muxer mux;
        channels channs(mp);
        pair<channelobptr, channelobptr> xy(plainpair(mp, mux));

        wr_ = getid(xy.second);
        insertchannel(channs, xy.second);
        seteventmask(xy.second, AUG_FDEVENTWR);

        rd_ = getid(xy.first);
        insertchannel(channs, xy.first);
        seteventmask(xy.first, AUG_FDEVENTRD);

        cout << "write: " << wr_ << endl;
        cout << "read: " << rd_ << endl;

        waitfdevents(mux);
        foreachchannel(channs, channelcb<cb>);

    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
