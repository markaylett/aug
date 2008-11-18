/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGSERV_BUILD
#include "augserv/main.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augserv/base.h"
#include "augserv/control.h"
#include "augserv/daemon.h"
#include "augserv/options.h"

#include "augutil/log.h"

#include "augsys/utility.h" /* aug_perrinfo() */
#include "augsys/windows.h" /* GetStdHandle() */

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augext/log.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>         /* EXIT_FAILURE */

static jmp_buf mark_;

static void
die_(const char* s)
{
    aug_perrinfo(aug_tlx, s, NULL);
    longjmp(mark_, 1);
}

static void
foreground_(void)
{
    aug_result result;
    if (AUG_ISFAIL(aug_initserv()))
        die_("aug_initserv() failed");

    result = aug_runserv();
    aug_termserv();
    if (AUG_ISFAIL(result))
        die_("aug_runserv() failed");
}

static void
daemonise_(void)
{
    aug_result result = aug_daemonise();

    if (AUG_ISNONE(result))
        foreground_();
    else if (AUG_ISFAIL(result))
        die_("aug_daemonise() failed");
}

#if defined(_WIN32)
static void
start_(void)
{
    if (AUG_ISFAIL(aug_start()))
        die_("aug_start() failed");
}
#endif /* _WIN32 */

static void
control_(int sig)
{
    if (AUG_ISFAIL(aug_control(sig)))
        die_("aug_control() failed");
}

static void
install_(void)
{
    if (AUG_ISFAIL(aug_install()))
        die_("aug_install() failed");
}

static void
uninstall_(void)
{
    if (AUG_ISFAIL(aug_uninstall()))
        die_("aug_uninstall() failed");
}

AUGSERV_API int
aug_main(int argc, char* argv[], const struct aug_service* service, void* arg)
{
    struct aug_options options;
    int daemon = 0, jmpret = setjmp(mark_);
    if (jmpret)
        return jmpret;

    aug_setservice_(service, arg);

#if defined(_WIN32)

    /* Assume that if there is no stdin handle then the process is being
       started by the Service Control Manager. */

    if (!GetStdHandle(STD_INPUT_HANDLE)) {

        /* Note: aug_readopts() will be called from the main service
           function. */

        aug_setlog(aug_tlx, aug_getdaemonlog());
        daemonise_();
        return EXIT_SUCCESS;
    }
#endif /* _WIN32 */

    if (AUG_ISFAIL(aug_readopts(&options, argc, argv))) {
        aug_perrinfo(aug_tlx, "getreadopts() failed", NULL);
        return EXIT_FAILURE;
    }

    if (AUG_CMDEXIT == options.command_)
        return EXIT_SUCCESS;

#if !defined(_WIN32)
    if (AUG_CMDSTART == options.command_) {

        /* Install daemon logger prior to opening log file. */

        aug_setlog(aug_tlx, aug_getdaemonlog());
        daemon = 1;
    }
#endif /* !_WIN32 */

    if (AUG_ISFAIL(aug_readservconf(*options.conffile_
                                    ? options.conffile_ : NULL,
                                    options.batch_, daemon)))
        die_("aug_readservconf() failed");

    switch (options.command_) {
    case AUG_CMDDEFAULT:
        foreground_();
        break;
    case AUG_CMDEXIT:
        assert(0);
    case AUG_CMDINSTALL:
        aug_ctxinfo(aug_tlx, "installing daemon process");
        install_();
        break;
    case AUG_CMDRECONF:
        aug_ctxinfo(aug_tlx, "re-configuring daemon process");
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
        aug_ctxinfo(aug_tlx, "getting status of daemon process");
        control_(AUG_EVENTSTATUS);
        break;
    case AUG_CMDSTOP:
        aug_ctxinfo(aug_tlx, "stopping daemon process");
        control_(AUG_EVENTSTOP);
        break;
    case AUG_CMDTEST:
        foreground_();
        break;
    case AUG_CMDUNINSTALL:
        aug_ctxinfo(aug_tlx, "uninstalling daemon process");
        uninstall_();
        break;
    }
    return EXIT_SUCCESS;
}
