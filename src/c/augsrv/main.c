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

#include "augutil/log.h"

#include "augsys/errinfo.h"
#include "augsys/log.h"

#include <assert.h>

static void
die_(const char* s)
{
    aug_perrinfo(s);
    aug_exitservice(1);
}

static void
foreground_(void)
{
    if (-1 == aug_initservice())
        die_("aug_initservice() failed");

    if (-1 == aug_runservice())
        die_("aug_runservice() failed");
}

static void
daemonise_(void)
{
    switch (aug_daemonise()) {
    case -1:
        die_("aug_daemonise() failed");
    case 0:
        break;
    case AUG_RETNONE:
        foreground_();
        break;
    default:
        assert(0);
    }
}

#if defined(_WIN32)
static void
start_(void)
{
    switch (aug_start()) {
    case -1:
        die_("aug_start() failed");
    case 0:
        break;
    default:
        assert(0);
    }
}
#endif /* _WIN32 */

static void
control_(int sigtype)
{
    switch (aug_control(sigtype)) {
    case -1:
        die_("aug_control() failed");
    case 0:
        break;
    default:
        assert(0);
    }
}

static void
install_(void)
{
    switch (aug_install()) {
    case -1:
        die_("aug_install() failed");
    case 0:
        break;
    default:
        assert(0);
    }
}

static void
uninstall_(void)
{
    switch (aug_uninstall()) {
    case -1:
        die_("aug_uninstall() failed");
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

    if (-1 == aug_readopts(&options, argc, argv))
        aug_exitservice(1);

    if (AUG_CMDEXIT == options.command_)
        aug_exitservice(0);

#if !defined(_WIN32)
    daemon = AUG_CMDSTART == options.command_;
#else /* _WIN32 */
    daemon = AUG_CMDDEFAULT == options.command_;
#endif /* !_WIN32 */

    /* Install daemon logger prior to opening log file. */

    if (daemon)
        aug_setlogger(aug_daemonlogger);

    if (-1 == aug_readserviceconf(options.confpath_, daemon))
        die_("aug_readserviceconf() failed");

    switch (options.command_) {
    case AUG_CMDDEFAULT:
#if !defined(_WIN32)
        foreground_();
#else /* _WIN32 */
        daemonise_();
#endif /* !_WIN32 */
        break;
    case AUG_CMDEXIT:
        assert(0);
    case AUG_CMDINSTALL:
        aug_info("installing daemon process");
        install_();
        break;
    case AUG_CMDRECONF:
        aug_info("re-configuring daemon process");
        control_(AUG_EVENTRECONF);
        break;
    case AUG_CMDSTART:
#if !defined(_WIN32)
        daemonise_();
#else /* _WIN32 */
        start_();
#endif /* _WIN32 */
        break;
    case AUG_CMDSTATUS:
        aug_info("getting status of daemon process");
        control_(AUG_EVENTSTATUS);
        break;
    case AUG_CMDSTOP:
        aug_info("stopping daemon process");
        control_(AUG_EVENTSTOP);
        break;
    case AUG_CMDTEST:
        foreground_();
        break;
    case AUG_CMDUNINSTALL:
        aug_info("uninstalling daemon process");
        uninstall_();
        break;
    }
    aug_exitservice(0);
}
