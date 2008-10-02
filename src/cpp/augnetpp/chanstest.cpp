/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augnetpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

namespace {

    const char ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    pair<chanptr, chanptr>
    plainpair(mpoolref mpool, aug_muxer_t muxer)
    {
        autosds sds(socketpair(AF_UNIX, SOCK_STREAM, 0));
        setnonblock(sds[0], true);
        setnonblock(sds[1], true);
        pair<chanptr, chanptr>
            p(make_pair(createplain(mpool, muxer, nextid(), sds[0],
                                    AUG_MDEVENTRD | AUG_MDEVENTEX),
                        createplain(mpool, muxer, nextid(), sds[1],
                                    AUG_MDEVENTWR | AUG_MDEVENTEX)));
        sds.release();
        return p;
    }

    unsigned rd_, wr_;
    int recv_ = 0;

    struct test {

        chanptr wrchan_;

        test()
            : wrchan_(null)
        {
        }
        void
        clearchan_(unsigned id) AUG_NOTHROW
        {
            aug_ctxinfo(aug_tlx, "clearing channel: %u", id);
            if (id == wr_)
                wrchan_ = null;
        }
        void
        errorchan_(unsigned id, const aug_errinfo& errinfo) AUG_NOTHROW
        {
            aug_perrinfo(aug_tlx, "socket error", &errinfo);
            exit(1);
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
                const char expect('A' + recv_++ % 26);
                if (ch != expect) {
                    aug_ctxerror(aug_tlx, "unexpected character '%c'", ch);
                    exit(1);
                }
                if ('Z' == ch && recv_ < 260)
                    setchanmask(wrchan_, AUG_MDEVENTRDWR);
            } else if (id == wr_) {
                write(stream, ALPHABET, 26);
                wrchan_ = object_cast<aug_chan>(stream);
                setchanmask(wrchan_, AUG_MDEVENTRD);
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

        scoped_chandler<test> chandler;
        muxer mux(mp);
        chans chans(mp, chandler);
        pair<chanptr, chanptr> xy(plainpair(mp, mux));

        wr_ = getchanid(xy.second);
        insertchan(chans, xy.second);

        rd_ = getchanid(xy.first);
        insertchan(chans, xy.first);

        while (recv_ < 26 * 10) {
            waitmdevents(mux);
            processchans(chans);
        }

        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
