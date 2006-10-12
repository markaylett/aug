/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/main.h"

static const char rcsid[] = "$Id$";

#include "augsrv/base.h"
#include "augsrv/control.h"
#include "augsrv/daemon.h"
#include "augsrv/options.h"
#include "augsrv/signal.h"
#include "augsrv/types.h"

#include "augutil/getopt.h"
#include "augutil/log.h"

#include "augsys/base.h"   /* aug_exit() */
#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/log.h"
#include "augsys/mplexer.h"
#include "augsys/unistd.h" /* aug_close() */

#include <assert.h>

/* closepipe_() should not be called from an atexit() handler: on Windows, the
   pipe is implemented as a socket pair.  The c-runtime may terminate the
   Winsock layer prior to calling the cleanup function. */

static void
closepipe_(void)
{
    int in = aug_eventin(), out = aug_eventout();
    if (-1 != in)
        AUG_PERRINFO(aug_close(in), "aug_close() failed");
    if (-1 != out)
        AUG_PERRINFO(aug_close(out), "aug_close() failed");
}

static void
die_(const char* s)
{
    aug_perrinfo(s);
    closepipe_();
    aug_exit(1);
}

static void
handler_(int sig)
{
    struct aug_event event;
    aug_info("handling interrupt");
    if (!aug_writeevent(aug_eventout(), aug_setsigevent(&event, sig)))
        aug_perrinfo("aug_writeevent() failed");
}

static void
openpipe_(void)
{
    int fds[2];

    if (-1 == aug_mplexerpipe(fds))
        die_("aug_mplexerpipe() failed");

    aug_seteventpipe_(fds);

    if (-1 == aug_signalhandler(handler_))
        die_("aug_signalhandler() failed");
}

static void
foreground_(const struct aug_service* service)
{
    if (-1 == service->init_(&service->arg_))
        die_("failed to initialise daemon");

    if (-1 == service->run_(&service->arg_))
        die_("failed to run daemon");
}

static void
daemonise_(const struct aug_service* service)
{
    switch (aug_daemonise(service)) {
    case -1:
        die_("failed to daemonise process");
    case 0:
        break;
    case AUG_RETNONE:
        foreground_(service);
        break;
    default:
        assert(0);
    }
}

#if defined(_WIN32)
static void
start_(const struct aug_service* service)
{
    switch (aug_start(service)) {
    case -1:
        die_("failed to control daemon process");
    case 0:
        break;
    default:
        assert(0);
    }
}
#endif /* _WIN32 */

static void
control_(const struct aug_service* service, int sigtype)
{
    switch (aug_control(service, sigtype)) {
    case -1:
        die_("failed to control daemon process");
    case 0:
        break;
    default:
        assert(0);
    }
}

static void
install_(const struct aug_service* service)
{
    switch (aug_install(service)) {
    case -1:
        die_("failed to install daemon");
    case 0:
        break;
    default:
        assert(0);
    }
}

static void
uninstall_(const struct aug_service* service)
{
    switch (aug_uninstall(service)) {
    case -1:
        die_("failed to uninstall daemon");
    case 0:
        break;
    default:
        assert(0);
    }
}

AUGSRV_API void
aug_main(const struct aug_service* service, int argc, char* argv[])
{
    int daemon;
    struct aug_options options;

    aug_setservice_(service);
    if (-1 == aug_readopts(service, &options, argc, argv))
        aug_exit(1);

    if (AUG_CMDEXIT == options.command_)
        aug_exit(0);

#if !defined(_WIN32)
    daemon = AUG_CMDSTART == options.command_;
#else /* _WIN32 */
    daemon = AUG_CMDDEFAULT == options.command_;
#endif /* !_WIN32 */

    /* Install daemon logger prior to opening log file. */

    if (daemon)
        aug_setlogger(aug_daemonlogger);

    if (-1 == service->config_(&service->arg_, options.conffile_, daemon))
        die_("failed to read configuration");

    switch (options.command_) {
    case AUG_CMDDEFAULT:
        openpipe_();
#if !defined(_WIN32)
        foreground_(service);
#else /* _WIN32 */
        daemonise_(service);
#endif /* !_WIN32 */
        break;
    case AUG_CMDEXIT:
        assert(0);
    case AUG_CMDINSTALL:
        aug_info("installing daemon process");
        install_(service);
        break;
    case AUG_CMDRECONF:
        aug_info("re-configuring daemon process");
        control_(service, AUG_EVENTRECONF);
        break;
    case AUG_CMDSTART:
#if !defined(_WIN32)
        openpipe_();
        daemonise_(service);
#else /* _WIN32 */
        start_(service);
#endif /* _WIN32 */
        break;
    case AUG_CMDSTATUS:
        aug_info("getting status of daemon process");
        control_(service, AUG_EVENTSTATUS);
        break;
    case AUG_CMDSTOP:
        aug_info("stopping daemon process");
        control_(service, AUG_EVENTSTOP);
        break;
    case AUG_CMDTEST:
        openpipe_();
        foreground_(service);
        break;
    case AUG_CMDUNINSTALL:
        aug_info("uninstalling daemon process");
        uninstall_(service);
        break;
    }
    closepipe_();
    aug_exit(0);
}
