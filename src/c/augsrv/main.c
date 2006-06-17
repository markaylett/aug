/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/main.h"

#include "augsrv/control.h"
#include "augsrv/daemon.h"
#include "augsrv/global.h"
#include "augsrv/options.h"
#include "augsrv/signal.h"
#include "augsrv/types.h"

#include "augutil/getopt.h"
#include "augutil/log.h"

#include "augsys/defs.h"   /* AUG_VERIFY */
#include "augsys/errno.h"
#include "augsys/log.h"
#include "augsys/mplexer.h"
#include "augsys/string.h" /* aug_perror() */
#include "augsys/unistd.h" /* aug_close() */

#include <assert.h>
#include <stdlib.h>        /* exit() */

static void
die_(const char* s)
{
    aug_perror(s);
    exit(1);
}

static void
handler_(int i)
{
    aug_info("handling interrupt");
    if (-1 == aug_writesig(aug_tosignal(i)))
        aug_perror("aug_writesig() failed");
}

static void
closepipe_(void)
{
    AUG_VERIFY(aug_close(aug_sigin()), "aug_close() failed");
    AUG_VERIFY(aug_close(aug_sigout()), "aug_close() failed");
}

static void
openpipe_(void)
{
    int fds[2];

    if (-1 == aug_mplexerpipe(fds))
        die_("aug_mplexerpipe() failed");

    aug_setsigpipe_(fds);
    if (-1 == atexit(closepipe_))
        die_("atexit() failed");

    if (-1 == aug_sigactions(handler_))
        die_("aug_sigactions() failed");
}

static void
foreground_(const struct aug_service* service)
{
    if (-1 == (*service->init_)(service->arg_))
        die_("failed to initialise daemon");

    if (-1 == (*service->run_)(service->arg_))
        die_("failed to run daemon");
}

static void
daemonise_(const struct aug_service* service)
{
    switch (aug_daemonise(service)) {
    case AUG_FOREGROUND:
        foreground_(service);
    case 0:
        break;
    case -1:
        die_("failed to daemonise process");
    case AUG_EEXISTS:
        aug_info("daemon process already running");
        break;
    default:
        assert(0);
    }
}

#if defined(_WIN32)
static int
start_(const struct aug_service* service)
{
    switch (aug_start(service)) {
    case 0:
        break;
    case -1:
        aug_perror("failed to control daemon process");
        return -1;
    case AUG_EEXISTS:
        aug_info("daemon process already running");
        break;
    case AUG_ENOTEXISTS:
        aug_info("daemon not installed");
        break;
    default:
        assert(0);
    }
    return 0;
}
#endif /* _WIN32 */

static void
control_(const struct aug_service* service, enum aug_signal sig)
{
    switch (aug_control(service, sig)) {
    case 0:
        break;
    case -1:
        die_("failed to control daemon process");
    case AUG_EEXISTS:
        aug_info("daemon process already running");
        break;
    case AUG_ENOTEXISTS:
        aug_info("daemon process not running");
        break;
    default:
        assert(0);
    }
}

static void
install_(const struct aug_service* service)
{
    switch (aug_install(service)) {
    case 0:
        break;
    case -1:
        die_("failed to install daemon");
    case AUG_EEXISTS:
        aug_info("daemon already installed");
        break;
    default:
        assert(0);
    }
}

static void
uninstall_(const struct aug_service* service)
{
    switch (aug_uninstall(service)) {
    case 0:
        break;
    case -1:
        die_("failed to uninstall daemon");
    case AUG_ENOTEXISTS:
        aug_info("daemon not installed");
        break;
    default:
        assert(0);
    }
}

AUGSRV_API void
aug_main(const struct aug_service* service, int argc, char* argv[])
{
    int daemon, ret;
    aug_setservice_(service);

    switch (ret = aug_readopts(service, argc, argv)) {
    case AUG_CMDNONE:
        exit(0);
    case -1:
        exit(1);
    default:
        break;
    }

#if !defined(_WIN32)
    daemon = AUG_CMDSTART == ret;
#else /* _WIN32 */
    daemon = AUG_CMDDEFAULT == ret;
#endif /* !_WIN32 */

    /* Install daemon logger prior to opening log file. */

    if (daemon)
        aug_setlogger(aug_daemonlogger);

    if (-1 == (*service->config_)(service->arg_, daemon))
        die_("failed to read configuration");

    switch (ret) {
    case AUG_CMDDEFAULT:
        openpipe_();
#if !defined(_WIN32)
        foreground_(service);
#else /* _WIN32 */
        daemonise_(service);
#endif /* !_WIN32 */
        break;
    case AUG_CMDINSTALL:
        aug_info("installing daemon process");
        install_(service);
        break;
    case AUG_CMDRECONF:
        aug_info("re-configuring daemon process");
        control_(service, AUG_SIGRECONF);
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
        control_(service, AUG_SIGSTATUS);
        break;
    case AUG_CMDSTOP:
        aug_info("stopping daemon process");
        control_(service, AUG_SIGSTOP);
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
    exit(0);
}
