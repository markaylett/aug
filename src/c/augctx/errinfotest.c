/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augctx.h"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    aug_autobasictlx();

    aug_setposixerrinfo(aug_tlerr, __FILE__, 101, EINVAL);

    if (0 != strcmp(aug_tlerr->file_, __FILE__)) {
        fprintf(stderr, "unexpected aug_errfile value: %s\n",
                aug_tlerr->file_);
        return 1;
    }

    if (101 != aug_tlerr->line_) {
        fprintf(stderr, "unexpected aug_errline value: %d\n",
                (int)aug_tlerr->line_);
        return 1;
    }

    if (0 != strcmp(aug_tlerr->src_, "posix")) {
        fprintf(stderr, "unexpected aug_errsrc value: %d\n",
                (int)aug_tlerr->src_);
        return 1;
    }

    if (0 != strcmp(aug_tlerr->desc_, strerror(EINVAL))) {
        fprintf(stderr, "unexpected aug_errdesc value: %s\n",
                aug_tlerr->desc_);
        return 1;
    }

    return 0;
}
