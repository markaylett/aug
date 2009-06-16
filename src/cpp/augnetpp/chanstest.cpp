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

namespace {

    const char ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    pair<chanptr, chanptr>
    plainpair(mpoolref mpool, aug_muxer_t muxer)
    {
        autosds sds(socketpair(AF_UNIX, SOCK_STREAM, 0));
        setnonblock(sds[0], true);
        setnonblock(sds[1], true);
        pair<chanptr, chanptr>
            p(make_pair(createplain(mpool, nextid(), muxer, sds[0],
                                    AUG_MDEVENTRDEX),
                        createplain(mpool, nextid(), muxer, sds[1],
                                    AUG_MDEVENTALL)));
        sds.release();
        return p;
    }

    aug_id rd_, wr_;
    int recv_ = 0;

    struct test : mpool_ops {

        chanptr wrchan_;

        test()
            : wrchan_(null)
        {
        }
        aug_bool
        authchan_(aug_id id, const char* subject,
                  const char* issuer) AUG_NOTHROW
        {
            return AUG_TRUE;
        }
        void
        clearchan_(aug_id id) AUG_NOTHROW
        {
            aug_ctxinfo(aug_tlx, "clearing channel: %d",
                        static_cast<int>(id));
            if (id == wr_)
                wrchan_ = null;
        }
        void
        errorchan_(chanref chan, const aug_errinfo& errinfo) AUG_NOTHROW
        {
            aug_perrinfo(aug_tlx, "socket error", &errinfo);
            exit(1);
        }
        aug_bool
        estabchan_(chanref chan, aug_id parent) AUG_NOTHROW
        {
            return AUG_TRUE;
        }
        aug_bool
        readychan_(chanref chan, unsigned short events) AUG_NOTHROW
        {
            const aug_id id(getchanid(chan));
            streamptr stream(object_cast<aug_stream>(chan));

            if (id == rd_) {
                char ch;
                read(stream, &ch, 1);
                const char expect('A' + recv_++ % 26);
                if (ch != expect) {
                    aug_ctxerror(aug_tlx, "unexpected character [%c]", ch);
                    exit(1);
                }
                if ('Z' == ch && recv_ < 260)
                    setchanwantwr(wrchan_, AUG_TRUE);
            } else if (id == wr_) {
                write(stream, ALPHABET, 26);
                wrchan_ = object_cast<aug_chan>(stream);
                setchanwantwr(wrchan_, AUG_FALSE);
            }
            return AUG_TRUE;
        }
    };
}

int
main(int argc, char* argv[])
{
    try {
        autotlx();

        mpoolptr mp(getmpool(aug_tlx));

        scoped_chandler_wrapper<test> chandler;
        muxer mux(mp);
        chans chans(mp, chandler);
        pair<chanptr, chanptr> xy(plainpair(mp, mux));

        wr_ = getchanid(xy.second);
        insertchan(chans, xy.second);

        rd_ = getchanid(xy.first);
        insertchan(chans, xy.first);

        while (recv_ < 26 * 10) {

            if (0 < getreadychans(chans))
                pollmdevents(mux);
            else
                waitmdevents(mux);

            processchans(chans);
        }

        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
