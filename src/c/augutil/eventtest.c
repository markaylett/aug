/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"
#include "augutil.h"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    int fds[2];
    struct aug_event in = { 1, 0 }, out = { !1, 0 };

    aug_atexitinit(&errinfo);

    if (-1 == aug_muxerpipe(fds)) {
        aug_perrinfo(NULL, "aug_term() failed");
        return 1;
    }

    if (!aug_writeevent(fds[1], &in)) {
        aug_perrinfo(NULL, "aug_writeevent() failed");
        goto fail;
    }

    if (!aug_readevent(fds[0], &out)) {
        aug_perrinfo(NULL, "aug_readevent() failed");
        goto fail;
    }

    if (in.type_ != out.type_) {
        fprintf(stderr, "unexpected event type from aug_readevent()\n");
        goto fail;
    }

    if (-1 == aug_close(fds[0]) || -1 == aug_close(fds[1])) {
        aug_perrinfo(NULL, "aug_close() failed");
        return 1;
    }

    return 0;

 fail:
    AUG_PERRINFO(aug_close(fds[0]), NULL, "aug_close() failed");
    AUG_PERRINFO(aug_close(fds[1]), NULL, "aug_close() failed");
    return 1;
}
