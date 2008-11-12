/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
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
        if (AUG_ISFAIL(aug_mclose(mds_[0])))
            aug_perrinfo(aug_tlx, "aug_mclose() failed", NULL);

    if (AUG_BADMD != mds_[1])
        if (AUG_ISFAIL(aug_mclose(mds_[1])))
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

static aug_result
openpipe_(void)
{
    aug_md mds[2];
    aug_result result;

    assert(AUG_BADMD == mds_[0] && AUG_BADMD == mds_[1]);

    if (AUG_ISFAIL(result = aug_muxerpipe(mds)))
        return result;

    mds_[0] = mds[0];
    mds_[1] = mds[1];

    if (AUG_ISFAIL(result = aug_setsighandler(sighandler_))) {
        closepipe_();
        return result;
    }

#if defined(_WIN32)
    SetConsoleCtrlHandler(ctrlhandler_, TRUE);
#endif /* _WIN32 */
    return AUG_SUCCESS;
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

AUGSRV_API aug_result
aug_readserviceconf(const char* conffile, int batch, int daemon)
{
    assert(service_.readconf_);
    return service_.readconf_(arg_, conffile, batch, daemon);
}

AUGSRV_API aug_result
aug_initservice(void)
{
    aug_result result;
    assert(service_.init_);

    if (AUG_ISFAIL(result = openpipe_()))
        return result;

    /* Flush pending writes to main memory: when init_() is called, the
       gaurantee of interactions exclusively with the main thread are lost. */

    AUG_WMB();

    if (AUG_ISFAIL(result = service_.init_(arg_))) {
        closepipe_();
        return result;
    }

    return AUG_SUCCESS;
}

AUGSRV_API aug_result
aug_runservice(void)
{
    assert(service_.run_);
    return service_.run_(arg_);
}

AUGSRV_API void
aug_termservice(void)
{
    if (AUG_BADMD != mds_[0]) {
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
