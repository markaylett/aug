/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/base.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsrv/signal.h"

#include "augsys/barrier.h"
#include "augsys/base.h"    /* aug_exit() */
#include "augsys/errinfo.h"
#include "augsys/log.h"
#include "augsys/muxer.h"
#include "augsys/unistd.h"  /* aug_close() */
#include "augsys/utility.h" /* aug_perrinfo() */
#include "augsys/windows.h"

#include "augutil/event.h"  /* struct aug_event */

#include <assert.h>
#include <stdlib.h>         /* NULL */
#include <string.h>         /* memcpy() */

/* No protection is required around these statics: they are set once from
   aug_main().

   On Windows, the Service Manager calls the service entry point on a separate
   thread: automatic variables on the main thread's stack will not be visible
   from the service thread. */

static struct aug_service service_ = { 0 };
static void* arg_ = NULL;
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
signalhandler_(int sig)
{
    struct aug_event event;
    aug_info("handling signal interrupt");
    if (!aug_writeevent(fds_[1], aug_setsigevent(&event, sig)))
        aug_perrinfo(NULL, "aug_writeevent() failed");
}

#if defined(_WIN32)
static BOOL WINAPI
ctrlhandler_(DWORD ctrl)
{
    struct aug_event event = { AUG_EVENTSTOP, NULL };
    aug_info("handling console interrupt");
    if (!aug_writeevent(fds_[1], &event))
        aug_perrinfo(NULL, "aug_writeevent() failed");
    return TRUE;
}
#endif /* _WIN32 */

static int
openpipe_(void)
{
    int fds[2];
    assert(-1 == fds_[0] && -1 == fds_[1]);

    if (-1 == aug_muxerpipe(fds))
        return -1;

    fds_[0] = fds[0];
    fds_[1] = fds[1];

    if (-1 == aug_signalhandler(signalhandler_)) {
        closepipe_();
        return -1;
    }

#if defined(_WIN32)
    SetConsoleCtrlHandler(ctrlhandler_, TRUE);
#endif /* _WIN32 */
    return 0;
}

AUG_EXTERNC void
aug_setservice_(const struct aug_service* service, void* arg)
{
    memcpy(&service_, service, sizeof(service_));
    arg_ = arg;
}

AUGSRV_API const char*
aug_getserviceopt(enum aug_option opt)
{
    assert(service_.getopt_);
    return service_.getopt_(arg_, opt);
}

AUGSRV_API int
aug_readserviceconf(const char* conffile, int batch, int daemon)
{
    assert(service_.readconf_);
    return service_.readconf_(arg_, conffile, batch, daemon);
}

AUGSRV_API int
aug_initservice(void)
{
    assert(service_.init_);
    if (-1 == openpipe_())
        return -1;

    /* Flush pending writes to main memory: when init_() the gaurantee of
       interactions exclusively with the main thread are lost. */

    AUG_WMB();

    if (-1 == service_.init_(arg_)) {
        closepipe_();
        return -1;
    }

    return 0;
}

AUGSRV_API int
aug_runservice(void)
{
    assert(service_.run_);
    return service_.run_(arg_);
}

AUGSRV_API void
aug_termservice(void)
{
    if (-1 != fds_[0]) {
        assert(service_.term_);
        service_.term_(arg_);
        closepipe_();
    }
}

AUGSRV_API int
aug_eventrd(void)
{
    return fds_[0];
}

AUGSRV_API int
aug_eventwr(void)
{
    return fds_[1];
}
