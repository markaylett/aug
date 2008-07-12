/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"
#include "augctx.h"

#include <fcntl.h>
#include <stdio.h>

int
main(int argc, char* argv[])
{
    aug_fd fd;
    aug_muxer_t muxer;
    aug_mpool* mpool;
    aug_chan* chan;
    aug_stream* stream;
    if (aug_initbasictlx() < 0)
        return 1;

    fd = aug_fopen("filetest.txt", O_CREAT | O_TRUNC | O_RDWR, 0666);
    aug_check(AUG_BADFD != fd);

    muxer = aug_createmuxer();
    aug_check(muxer);

    mpool = aug_getmpool(aug_tlx);
    aug_check(mpool);

    chan = aug_createfile(mpool, fd, "chantest", muxer);
    aug_check(chan);

    stream = aug_cast(chan, aug_streamid);
    aug_release(chan);

    aug_write(stream, "test\n", 5);
    aug_release(stream);
    return 0;
}
