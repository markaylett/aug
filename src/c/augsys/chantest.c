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
#include "augsys.h"
#include "augctx.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> /* exit() */

static void
clean(void)
{
    remove("chantest.txt");
}

int
main(int argc, char* argv[])
{
    aug_fd fd;
    aug_muxer_t muxer;
    aug_mpool* mpool;
    aug_chan* chan;
    aug_stream* stream;

    if (!aug_autotlx())
        return 1;

    fd = aug_fopen("chantest.txt", O_CREAT | O_TRUNC | O_RDWR, 0666);
    atexit(clean);
    aug_check(AUG_BADFD != fd);

    mpool = aug_getmpool(aug_tlx);
    aug_check(mpool);

    muxer = aug_createmuxer(mpool);
    aug_check(muxer);

    chan = aug_createfile(mpool, "chantest", muxer, fd);
    aug_check(chan);

    aug_release(mpool);

    stream = aug_cast(chan, aug_streamid);
    aug_release(chan);

    aug_write(stream, "test\n", 5);
    aug_release(stream);
    return 0;
}
