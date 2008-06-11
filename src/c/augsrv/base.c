/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/base.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsrv/signal.h"

#include "augsys/barrier.h"
#include "augsys/muxer.h"
#include "augsys/unistd.h"  /* aug_fclose() */
#include "augsys/utility.h" /* aug_perrinfo() */
#include "augsys/windows.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augext/log.h"

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
static aug_md mds_[2] = { AUG_BADMD, AUG_BADMD };

/* closepipe_() should not be called from an atexit() handler: on Windows, the
   pipe is implemented as a socket pair.  The c-runtime may terminate the
   Winsock layer prior to calling the cleanup function. */

static void
closepipe_(void)
{
    if (AUG_BADMD != mds_[0])
        if (-1 == aug_mclose(mds_[0]))
            aug_perrinfo(aug_tlx, "aug_mclose() failed", NULL);

    if (AUG_BADMD != mds_[1])
        if (-1 == aug_mclose(mds_[1]))
            aug_perrinfo(aug_tlx, "aug_mclose() failed", NULL);

    mds_[0] = AUG_BADMD;
    mds_[1] = AUG_BADMD;
}

/* On Windows, signal handlers are not called on the main thread.  The main
   thread's context will, therefore, be unavailble. */

static void
sighandler_(int sig)
{
    struct aug_event event;
    if (!aug_writeevent(mds_[1], aug_setsigevent(&event, sig)))
        abort();
}

#if defined(_WIN32)
static BOOL WINAPI
ctrlhandler_(DWORD ctrl)
{
    struct aug_event event = { AUG_EVENTSTOP, NULL };
    if (!aug_writeevent(mds_[1], &event))
        abort();
    return TRUE;
}
#endif /* _WIN32 */

static int
openpipe_(void)
{
    aug_md mds[2];
    assert(AUG_BADMD == mds_[0] && AUG_BADMD == mds_[1]);

    if (-1 == aug_muxerpipe(mds))
        return -1;

    mds_[0] = mds[0];
    mds_[1] = mds[1];

    if (-1 == aug_setsighandler(sighandler_)) {
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
    if (-1 != mds_[0]) {
        assert(service_.term_);
        service_.term_(arg_);
        closepipe_();
    }
}

AUGSRV_API aug_md
aug_eventrd(void)
{
    return mds_[0];
}

AUGSRV_API aug_md
aug_eventwr(void)
{
    return mds_[1];
}
