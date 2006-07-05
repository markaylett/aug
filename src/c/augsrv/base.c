/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/base.h"

static const char rcsid[] = "$Id:$";

#include "augsrv/types.h"

#include <stdlib.h> /* NULL */
#include <string.h> /* memcpy() */

/* No protection is required around these statics: they are only set once,
   from aug_main(). */

/* On Windows, the Service Manager calls the service entry point on a separate
   thread - automatic variables on the main thread's stack will not be visible
   from the service thread. */

static struct aug_service service_ = { 0 };
static int fds_[2] = { -1, -1 };

AUGSRV_EXTERN void
aug_setservice_(const struct aug_service* service)
{
    memcpy(&service_, service, sizeof(service_));
}

AUGSRV_EXTERN void
aug_seteventpipe_(int fds[2])
{
    fds_[0] = fds[0];
    fds_[1] = fds[1];
}

AUGSRV_API const struct aug_service*
aug_service(void)
{
    return &service_;
}

AUGSRV_API int
aug_eventin(void)
{
    return fds_[0];
}

AUGSRV_API int
aug_eventout(void)
{
    return fds_[1];
}
