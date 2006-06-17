/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
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
    aug_signal_t in = 1, out = !1;

    aug_atexitinit(&errinfo);

    if (-1 == aug_mplexerpipe(fds)) {
        aug_perrinfo("aug_term() failed");
        return 1;
    }

    if (-1 == aug_writesignal(fds[1], in)) {
        aug_perrinfo("aug_writesignal() failed");
        goto fail;
    }

    if (-1 == aug_readsignal(fds[0], &out)) {
        aug_perrinfo("aug_readsignal() failed");
        goto fail;
    }

    if (in != out) {
        fprintf(stderr, "unexpected signal value from aug_readsignal()\n");
        goto fail;
    }

    if (-1 == aug_close(fds[0]) || -1 == aug_close(fds[1])) {
        aug_perrinfo("aug_close() failed");
        return 1;
    }

    return 0;

 fail:
    AUG_PERRINFO(aug_close(fds[0]), "aug_close() failed");
    AUG_PERRINFO(aug_close(fds[1]), "aug_close() failed");
    return 1;
}
