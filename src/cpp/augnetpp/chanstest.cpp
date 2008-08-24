/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augnetpp.hpp"
#include "augsyspp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

namespace {

    pair<chanptr, chanptr>
    plainpair(mpoolref mpool, aug_muxer_t muxer)
    {
        autosds sds(socketpair(AF_UNIX, SOCK_STREAM, 0));
        setnonblock(sds[0], true);
        setnonblock(sds[1], true);
        return make_pair(createplain(mpool, muxer, nextid(), sds[0],
                                     AUG_MDEVENTRD),
                         createplain(mpool, muxer, nextid(), sds[1],
                                     AUG_MDEVENTWR));
    }

    unsigned rd_, wr_;
    bool done_ = false;

    struct test {

        chandler<test> chandler_;
        chans chans_;

        explicit
        test(mpoolref mpool)
            : chans_(null)
        {
            chandler_.reset(this);
            chans tmp(getmpool(aug_tlx), chandler_);
            chans_.swap(tmp);
        }
        smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (equalid<aug_object>(id) || equalid<aug_chandler>(id))
                return object_retain<aug_object>(chandler_);
            return null;
        }
        void
        retain_() AUG_NOTHROW
        {
        }
        void
        release_() AUG_NOTHROW
        {
        }
        void
        clearchan_(unsigned id) AUG_NOTHROW
        {
        }
        aug_bool
        estabchan_(unsigned id, obref<aug_stream> stream,
                   unsigned parent) AUG_NOTHROW
        {
            return AUG_TRUE;
        }
        aug_bool
        readychan_(unsigned id, obref<aug_stream> stream,
                   unsigned short events) AUG_NOTHROW
        {
            if (id == rd_) {
                char ch;
                read(stream, &ch, 1);
                if ('A' == ch)
                    done_ = true;
            } else if (id == wr_) {
                write(stream, "A", 1);
            }
            return AUG_TRUE;
        }
    };
}

int
main(int argc, char* argv[])
{
    try {
        autobasictlx();

        mpoolptr mp(getmpool(aug_tlx));

        chandler<test> chandler;
        muxer mux(mp);
        chans chans(mp, chandler);
        pair<chanptr, chanptr> xy(plainpair(mp, mux));

        wr_ = getchanid(xy.second);
        insertchan(chans, xy.second);

        rd_ = getchanid(xy.first);
        insertchan(chans, xy.first);

        while (!done_) {
            waitmdevents(mux);
            processchans(chans);
        }

        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
