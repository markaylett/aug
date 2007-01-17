/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/base.h"

static const char rcsid[] = "$Id$";

#include "augsrv/signal.h"

#include "augsys/base.h"   /* aug_exit() */
#include "augsys/errinfo.h"
#include "augsys/log.h"
#include "augsys/mplexer.h"
#include "augsys/unistd.h" /* aug_close() */

#include "augutil/event.h" /* struct aug_event */

#include <assert.h>
#include <stdlib.h>        /* NULL */
#include <string.h>        /* memcpy() */

/* No protection is required around these statics: they are only set once,
   from aug_main().

   On Windows, the Service Manager calls the service entry point on a separate
   thread: automatic variables on the main thread's stack will not be visible
   from the service thread. */

static struct aug_service service_ = { 0 };
static int fds_[2] = { -1, -1 };

/* closepipe_() should not be called from an atexit() handler: on Windows, the
   pipe is implemented as a socket pair.  The c-runtime may terminate the
   Winsock layer prior to calling the cleanup function. */

static void
closepipe_(void)
{
    if (-1 != fds_[0])
        AUG_PERRINFO(aug_close(fds_[0]), NULL, "aug_close() failed");
    if (-1 != fds_[1])
        AUG_PERRINFO(aug_close(fds_[1]), NULL, "aug_close() failed");

    fds_[0] = -1;
    fds_[1] = -1;
}

static void
handler_(int sig)
{
    struct aug_event event;
    aug_info("handling interrupt");
    if (!aug_writeevent(fds_[1], aug_setsigevent(&event, sig)))
        aug_perrinfo(NULL, "aug_writeevent() failed");
}

static int
openpipe_(void)
{
    int fds[2];
    assert(-1 == fds_[0] && -1 == fds_[1]);

    if (-1 == aug_mplexerpipe(fds))
        return -1;

    fds_[0] = fds[0];
    fds_[1] = fds[1];

    if (-1 == aug_signalhandler(handler_)) {
        closepipe_();
        return -1;
    }

    return 0;
}

AUGSRV_EXTERN void
aug_setservice_(const struct aug_service* service)
{
    memcpy(&service_, service, sizeof(service_));
}

AUGSRV_API const char*
aug_getserviceopt(enum aug_option opt)
{
    assert(service_.getopt_);
    return service_.getopt_(&service_.arg_, opt);
}

AUGSRV_API int
aug_readserviceconf(const char* conffile, int daemon)
{
    assert(service_.readconf_);
    return service_.readconf_(&service_.arg_, conffile, daemon);
}

AUGSRV_API int
aug_initservice(void)
{
    assert(service_.init_);
    if (-1 == openpipe_())
        return -1;

    if (-1 == service_.init_(&service_.arg_)) {
        closepipe_();
        return -1;
    }

    return 0;
}

AUGSRV_API int
aug_runservice(void)
{
    assert(service_.run_);
    return service_.run_(&service_.arg_);
}

AUGSRV_API void
aug_termservice(void)
{
    if (-1 != fds_[0]) {
        assert(service_.term_);
        service_.term_(&service_.arg_);
        closepipe_();
    }
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
