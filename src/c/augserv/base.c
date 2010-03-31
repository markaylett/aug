/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define AUGSERV_BUILD
#include "augserv/base.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augserv/signal.h"
#include "augserv/types.h"

#include "augsys/muxer.h"
#include "augsys/unistd.h"  /* aug_fclose() */
#include "augsys/utility.h" /* aug_perrinfo() */
#include "augsys/windows.h"

#include "augctx/atomic.h"
#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augext/log.h"
#include "augext/task.h"

#include <assert.h>
#include <stdlib.h>         /* NULL */
#include <string.h>         /* memcpy() */

/* No protection is required around these statics: they are set once from
   aug_main().

   Taking a copy of the service structure allows aug_setserv_() to be called
   with an automatic variable pointer. */

static struct aug_serv serv_ = { 0 };
static aug_events_t events_ = NULL;
static aug_task* task_ = 0;

/* destroyevents_() should not be called from an atexit() handler: on Windows,
   the event pipe is implemented as a socket pair.  The c-runtime may
   terminate the Winsock layer prior to calling the cleanup function. */

static void
destroyevents_(void)
{
    if (events_) {

        /* Always restore default signal handlers before destroying events. */

        aug_setsighandler(0);
        aug_destroyevents(events_);
        events_ = NULL;
    }
}

/* On Windows, signal handlers are not called on the main thread.  The main
   thread's context will, therefore, be unavailble. */

static void
sighandler_(int sig)
{
    struct aug_event event;
    aug_result result = aug_writeevent_A(events_,
                                         aug_sigtoevent(sig, &event));

    /* The signal is ignored if the write fails with EAGAIN/EWOULDBLOCK.  This
       could happen if the event pipe is full.  What else can one do? */

    if (result < 0 && AUG_EXBLOCK != aug_getexcept(aug_tlx))
        abort();
}

#if defined(_WIN32)
static BOOL WINAPI
ctrlhandler_(DWORD ctrl)
{
    struct aug_event event = { AUG_EVENTSTOP, NULL };
    aug_result result = aug_writeevent_A(events_, &event);

    if (result < 0 && AUG_EXBLOCK != aug_getexcept(aug_tlx))
        abort();

    return TRUE;
}
#endif /* _WIN32 */

static aug_result
createevents_AIN(void)
{
    aug_mpool* mpool;

    assert(!events_);

    mpool = aug_getmpool(aug_tlx);
    events_ = aug_createevents_AIN(mpool);
    aug_release(mpool);

    if (!events_)
        return -1;

    if (aug_setsighandler(sighandler_) < 0) {
        destroyevents_();
        return -1;
    }

#if defined(_WIN32)
    SetConsoleCtrlHandler(ctrlhandler_, TRUE);
#endif /* _WIN32 */
    return 0;
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
aug_initserv_AIN(void)
{
    assert(serv_.create_);
    assert(!task_);

    if (createevents_AIN() < 0)
        return -1;

    /* Flush pending writes to main memory: when init_() is called, the
       gaurantee of interactions exclusively with the main thread are lost. */

    aug_wmb();

    if (!(task_ = (*serv_.create_)())) {
        destroyevents_();
        return -1;
    }

    return 0;
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
    if (events_) {
        assert(task_);
        aug_release(task_);
        destroyevents_();
    }
}

AUGSERV_API aug_events_t
aug_events(void)
{
    return events_;
}
