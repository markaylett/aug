/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGSERV_BUILD
#include "augserv/base.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augserv/signal.h"
#include "augserv/types.h"

#include "augsys/barrier.h"
#include "augsys/muxer.h"
#include "augsys/unistd.h"  /* aug_fclose() */
#include "augsys/utility.h" /* aug_perrinfo() */
#include "augsys/windows.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augext/log.h"
#include "augext/task.h"

#include "augutil/event.h"  /* struct aug_event */

#include <assert.h>
#include <stdlib.h>         /* NULL */
#include <string.h>         /* memcpy() */

/* No protection is required around these statics: they are set once from
   aug_main().

   Taking a copy of the service structure allows aug_setserv_() to be called
   with an automatic variable pointer. */

static struct aug_serv serv_ = { 0 };
static aug_md mds_[2] = { AUG_BADMD, AUG_BADMD };
static aug_task* task_ = 0;

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
aug_setserv_(const struct aug_serv* serv)
{
    memcpy(&serv_, serv, sizeof(serv_));
}

AUGSERV_API const char*
aug_getservopt(int opt)
{
    assert(serv_.getopt_);
    return (*serv_.getopt_)(opt);
}

AUGSERV_API aug_result
aug_readservconf(const char* conffile, aug_bool batch, aug_bool daemon)
{
    assert(serv_.readconf_);
    return (*serv_.readconf_)(conffile, batch, daemon);
}

AUGSERV_API aug_result
aug_initserv(void)
{
    aug_result result;
    assert(serv_.create_);
    assert(!task_);

    if (AUG_ISFAIL(result = openpipe_()))
        return result;

    /* Flush pending writes to main memory: when init_() is called, the
       gaurantee of interactions exclusively with the main thread are lost. */

    AUG_WMB();

    if (!(task_ = (*serv_.create_)())) {
        closepipe_();
        return AUG_FAILERROR;
    }

    return AUG_SUCCESS;
}

AUGSERV_API aug_result
aug_runserv(void)
{
    assert(task_);
    return aug_runtask(task_);
}

AUGSERV_API void
aug_termserv(void)
{
    if (AUG_BADMD != mds_[0]) {
        assert(task_);
        aug_release(task_);
        closepipe_();
    }
}

AUGSERV_API aug_md
aug_eventrd(void)
{
    return mds_[0];
}

AUGSERV_API aug_md
aug_eventwr(void)
{
    return mds_[1];
}
