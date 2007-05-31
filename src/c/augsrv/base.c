/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/base.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsrv/signal.h"

#include "augsys/base.h"   /* aug_exit() */
#include "augsys/errinfo.h"
#include "augsys/log.h"
#include "augsys/mplexer.h"
#include "augsys/unistd.h" /* aug_close() */
#include "augsys/windows.h"

#include "augutil/event.h" /* struct aug_event */

#include <assert.h>
#include <stdlib.h>        /* NULL */
#include <string.h>        /* memcpy() */

/* No protection is required around these statics: they are only set once,
   from aug_main().

   On Windows, the Service Manager calls the service entry point on a separate
   thread: automatic variables on the main thread's stack will not be visible
   from the service thread. */

static struct aug_server server_ = { 0 };
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
    struct aug_event event = { AUG_EVENTSTOP, AUG_VARNULL };
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

    if (-1 == aug_mplexerpipe(fds))
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
aug_setserver_(const struct aug_server* server, void* arg)
{
    memcpy(&server_, server, sizeof(server_));
    arg_ = arg;
}

AUGSRV_API const char*
aug_getserveropt(enum aug_option opt)
{
    assert(server_.getopt_);
    return server_.getopt_(arg_, opt);
}

AUGSRV_API int
aug_readserverconf(const char* conffile, int prompt, int daemon)
{
    assert(server_.readconf_);
    return server_.readconf_(arg_, conffile, prompt, daemon);
}

AUGSRV_API int
aug_initserver(void)
{
    assert(server_.init_);
    if (-1 == openpipe_())
        return -1;

    if (-1 == server_.init_(arg_)) {
        closepipe_();
        return -1;
    }

    return 0;
}

AUGSRV_API int
aug_runserver(void)
{
    assert(server_.run_);
    return server_.run_(arg_);
}

AUGSRV_API void
aug_termserver(void)
{
    if (-1 != fds_[0]) {
        assert(server_.term_);
        server_.term_(arg_);
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
