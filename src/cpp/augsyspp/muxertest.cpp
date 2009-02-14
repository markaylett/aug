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
