/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"

#define MSG1_ "first chunk, "
#define MSG2_ "second chunk"

#include <stdio.h>

static void
test(aug_muxer_t muxer, int n)
{
    int sv[2];
    struct iovec iov[2];
    char buf[AUG_MAXLINE];

    if (0 == n)
        return;

    if (-1 == aug_socketpair(AF_UNIX, SOCK_STREAM, 0, sv)) {
        aug_perrinfo(NULL, "aug_socketpair() failed");
        exit(1);
    }

    iov[0].iov_base = MSG1_;
    iov[0].iov_len = sizeof(MSG1_) - 1;
    iov[1].iov_base = MSG2_;
    iov[1].iov_len = sizeof(MSG2_);

    if (-1 == aug_setfdeventmask(muxer, sv[0], AUG_FDEVENTRDWR)
        || -1 == aug_setfdeventmask(muxer, sv[1], AUG_FDEVENTRD)) {
        aug_perrinfo(NULL, "aug_setfdeventmask() failed");
        exit(1);
    }

    if (-1 == aug_writev(sv[0], iov, 2)) {
        aug_perrinfo(NULL, "aug_writev() failed");
        exit(1);
    }

    if (-1 == aug_waitfdevents(muxer, NULL)) {
        aug_writelog(AUG_LOGINFO, "sv[0]=[%d], sv[1]=[%d]", sv[0], sv[1]);
        aug_perrinfo(NULL, "aug_waitfdevents() failed");
        exit(1);
    }

    test(muxer, n - 1);

    if (-1 == aug_read(sv[1], buf, iov[0].iov_len + iov[1].iov_len)) {
        aug_perrinfo(NULL, "aug_read() failed");
        exit(1);
    }

    if (0 != strcmp(buf, MSG1_ MSG2_)) {
       fprintf(stderr, "unexpected buffer contents: %s\n", buf);
       exit(1);
    }

    if (-1 == aug_setfdeventmask(muxer, sv[0], 0)
        || -1 == aug_setfdeventmask(muxer, sv[1], 0)) {
        aug_perrinfo(NULL, "aug_setfdeventmask() failed");
        exit(1);
    }

    aug_close(sv[0]);
    aug_close(sv[1]);
}

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    aug_muxer_t muxer;
    aug_atexitinit(&errinfo);
    muxer = aug_createmuxer();
    test(muxer, 30);
    aug_destroymuxer(muxer);
    return 0;
}
