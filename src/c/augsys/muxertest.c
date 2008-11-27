/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augsys.h"
#include "augctx.h"

#define MSG1_ "first chunk, "
#define MSG2_ "second chunk"

#include <stdio.h>
#include <stdlib.h> /* exit() */

static void
test(aug_muxer_t muxer, int n)
{
    aug_sd sv[2];
    struct iovec iov[2];
    char buf[AUG_MAXLINE];

    if (0 == n)
        return;

    if (AUG_ISFAIL(aug_socketpair(AF_UNIX, SOCK_STREAM, 0, sv))) {
        aug_perrinfo(aug_tlx, "aug_socketpair() failed", NULL);
        exit(1);
    }

    iov[0].iov_base = MSG1_;
    iov[0].iov_len = sizeof(MSG1_) - 1;
    iov[1].iov_base = MSG2_;
    iov[1].iov_len = sizeof(MSG2_);

    if (AUG_ISFAIL(aug_setmdeventmask(muxer, sv[0], AUG_MDEVENTALL))
        || AUG_ISFAIL(aug_setmdeventmask(muxer, sv[1], AUG_MDEVENTRDEX))) {
        aug_perrinfo(aug_tlx, "aug_setmdeventmask() failed", NULL);
        exit(1);
    }

    if (AUG_MDEVENTALL != aug_getmdeventmask(muxer, sv[0])
        || AUG_MDEVENTRDEX != aug_getmdeventmask(muxer, sv[1])) {
        aug_perrinfo(aug_tlx, "aug_getmdeventmask() failed", NULL);
        exit(1);
    }

    if (AUG_ISFAIL(aug_swritev(sv[0], iov, 2))) {
        aug_perrinfo(aug_tlx, "aug_writev() failed", NULL);
        exit(1);
    }

    if (AUG_ISFAIL(aug_waitmdevents(muxer, NULL))) {
        aug_perrinfo(aug_tlx, "aug_waitmdevents() failed", NULL);
        exit(1);
    }

    test(muxer, n - 1);

    if (AUG_ISFAIL(aug_sread(sv[1], buf, iov[0].iov_len + iov[1].iov_len))) {
        aug_perrinfo(aug_tlx, "aug_sread() failed", NULL);
        exit(1);
    }

    if (0 != strcmp(buf, MSG1_ MSG2_)) {
       fprintf(stderr, "unexpected buffer contents: %s\n", buf);
       exit(1);
    }

    if (AUG_ISFAIL(aug_setmdeventmask(muxer, sv[0], 0))
        || AUG_ISFAIL(aug_setmdeventmask(muxer, sv[1], 0))) {
        aug_perrinfo(aug_tlx, "aug_setmdeventmask() failed", NULL);
        exit(1);
    }

    aug_sclose(sv[0]);
    aug_sclose(sv[1]);
}

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    aug_muxer_t muxer;
    if (!aug_autodltlx())
        return 1;

    mpool = aug_getmpool(aug_tlx);
    muxer = aug_createmuxer(mpool);
    aug_release(mpool);

    test(muxer, 30);
    aug_destroymuxer(muxer);
    return 0;
}
