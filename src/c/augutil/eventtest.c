/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augutil.h"
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    aug_sd sds[2];
    struct aug_event in = { 1, 0 }, out = { !1, 0 };

    if (!aug_autotlx())
        return 1;

    if (AUG_ISFAIL(aug_muxerpipe(sds))) {
        aug_perrinfo(aug_tlx, "aug_term() failed", NULL);
        return 1;
    }

    /* Sticky events not required for fixed length blocking read. */

    if (!aug_writeevent(sds[1], &in)) {
        aug_perrinfo(aug_tlx, "aug_writeevent() failed", NULL);
        goto fail;
    }

    if (!aug_readevent(sds[0], &out)) {
        aug_perrinfo(aug_tlx, "aug_readevent() failed", NULL);
        goto fail;
    }

    if (in.type_ != out.type_) {
        fprintf(stderr, "unexpected event type from aug_readevent()\n");
        goto fail;
    }

    if (AUG_ISFAIL(aug_sclose(sds[0])) || AUG_ISFAIL(aug_sclose(sds[1]))) {
        aug_perrinfo(aug_tlx, "aug_close() failed", NULL);
        return 1;
    }

    return 0;

 fail:
    aug_sclose(sds[0]);
    aug_sclose(sds[1]);
    return 1;
}
