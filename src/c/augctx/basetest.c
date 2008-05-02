/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctx.h"

#include <errno.h>

int
main(int argc, char* argv[])
{
    if (aug_start() < 0)
        return 1;
    aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, ENOMEM);
    aug_perrinfo(aug_tlx, "operation failed", NULL);
    return 0;
}
