/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"
#include "augctx.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> /* exit() */

int
main(int argc, char* argv[])
{
    aug_fd fd;
    aug_muxer_t muxer;
    aug_mpool* mpool;
    aug_chan* chan;
    aug_stream* stream;

    if (AUG_ISFAIL(aug_autobasictlx()))
        return 1;

    fd = aug_fopen("filetest.txt", O_CREAT | O_TRUNC | O_RDWR, 0666);
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
