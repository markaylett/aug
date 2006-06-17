/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/global.h"

static const char rcsid[] = "$Id:$";

#include <stdlib.h> /* NULL */

/* No protection is required around these statics: they are only set once,
   from aug_main(). */

static const struct aug_service* service_ = NULL;
static int fds_[2] = { -1, -1 };

AUGSRV_EXTERN void
aug_setservice_(const struct aug_service* service)
{
    service_ = service;
}

AUGSRV_EXTERN void
aug_setsignalpipe_(int fds[2])
{
    fds_[0] = fds[0];
    fds_[1] = fds[1];
}

AUGSRV_API const struct aug_service*
aug_service(void)
{
    return service_;
}

AUGSRV_API int
aug_signalin(void)
{
    return fds_[0];
}

AUGSRV_API int
aug_signalout(void)
{
    return fds_[1];
}
