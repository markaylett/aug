/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
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

    if (-1 == aug_socketpair(AF_UNIX, SOCK_STREAM, 0, sv)) {
        aug_perrinfo(aug_tlx, "aug_socketpair() failed", NULL);
        exit(1);
    }

    iov[0].iov_base = MSG1_;
    iov[0].iov_len = sizeof(MSG1_) - 1;
    iov[1].iov_base = MSG2_;
    iov[1].iov_len = sizeof(MSG2_);

    if (-1 == aug_setfdeventmask(muxer, sv[0], AUG_FDEVENTRDWR)
        || -1 == aug_setfdeventmask(muxer, sv[1], AUG_FDEVENTRD)) {
        aug_perrinfo(aug_tlx, "aug_setfdeventmask() failed", NULL);
        exit(1);
    }

    if (AUG_FDEVENTRDWR != aug_fdeventmask(muxer, sv[0])
        || AUG_FDEVENTRD != aug_fdeventmask(muxer, sv[1])) {
        aug_perrinfo(aug_tlx, "aug_fdeventmask() failed", NULL);
        exit(1);
    }

    if (-1 == aug_swritev(sv[0], iov, 2)) {
        aug_perrinfo(aug_tlx, "aug_writev() failed", NULL);
        exit(1);
    }

    if (-1 == aug_waitfdevents(muxer, NULL)) {
        aug_perrinfo(aug_tlx, "aug_waitfdevents() failed", NULL);
        exit(1);
    }

    test(muxer, n - 1);

    if (-1 == aug_sread(sv[1], buf, iov[0].iov_len + iov[1].iov_len)) {
        aug_perrinfo(aug_tlx, "aug_read() failed", NULL);
        exit(1);
    }

    if (0 != strcmp(buf, MSG1_ MSG2_)) {
       fprintf(stderr, "unexpected buffer contents: %s\n", buf);
       exit(1);
    }

    if (-1 == aug_setfdeventmask(muxer, sv[0], 0)
        || -1 == aug_setfdeventmask(muxer, sv[1], 0)) {
        aug_perrinfo(aug_tlx, "aug_setfdeventmask() failed", NULL);
        exit(1);
    }

    aug_sclose(sv[0]);
    aug_sclose(sv[1]);
}

int
main(int argc, char* argv[])
{
    aug_muxer_t muxer;
    if (aug_autobasictlx() < 0)
        return 1;

    muxer = aug_createmuxer();
    test(muxer, 30);
    aug_destroymuxer(muxer);
    return 0;
}
