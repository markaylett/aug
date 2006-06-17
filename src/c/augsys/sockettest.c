/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"

#define MSG1_ "first chunk, "
#define MSG2_ "second chunk"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;

    int sv[2];
    struct iovec iov[2];
    char buf[AUG_MAXLINE];

    aug_atexitinit(&errinfo);

    if (-1 == aug_socketpair(AF_UNIX, SOCK_STREAM, 0, sv)) {
        aug_perrinfo("aug_socketpair() failed");
        return 1;
    }

    iov[0].iov_base = MSG1_;
    iov[0].iov_len = sizeof(MSG1_) - 1;
    iov[1].iov_base = MSG2_;
    iov[1].iov_len = sizeof(MSG2_);

    if (-1 == aug_writev(sv[0], iov, 2)) {
        aug_perrinfo("aug_writev() failed");
        return 1;
    }

    if (-1 == aug_setnonblock(sv[1], 1)) {
        aug_perrinfo("aug_setnonblock() failed");
        return 1;
    }

    if (-1 == aug_read(sv[1], buf, iov[0].iov_len + iov[1].iov_len)) {
        aug_perrinfo("aug_read() failed");
        return 1;
    }

    if (0 != strcmp(buf, MSG1_ MSG2_)) {
       fprintf(stderr, "unexpected buffer contents: %s\n", buf);
       return 1;
    }

    aug_close(sv[0]);
    aug_close(sv[1]);
    return 0;
}
