/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augsyspp.hpp"
#include "augctxpp.hpp"

using namespace aug;
using namespace std;

int
main(int argc, char* argv[])
{
    try {
        autotlx();
        muxer mux(getmpool(aug_tlx));

        for (int i(0); i < 256; ++i) {

            autosds xy(socketpair(AF_UNIX, SOCK_STREAM, 0));

            setmdeventmask(mux, xy[0], AUG_MDEVENTRDEX);
            setmdeventmask(mux, xy[1], AUG_MDEVENTRDEX);

            setmdeventmask(mux, xy[1], 0);
            setmdeventmask(mux, xy[0], 0);
        }
        return 0;
    } AUG_PERRINFOCATCH;
    return 1;
}
