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

#include "augsys/utility.h" /* aug_perrinfo() */
#include "augsys/windows.h" /* GetStdHandle() */

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>         /* EXIT_FAILURE */

#define CONFFILE_(x) (*(x).confile_ ? (x).confile_ : NULL)

static jmp_buf mark_;

static void
die_(const char* s)
{
    aug_perrinfo(aug_tlx, s, NULL);
    longjmp(mark_, 1);
}

static void
foreground_(const struct aug_options* options)
{
    aug_result result;

    if (AUG_ISFAIL(aug_readservconf(AUG_CONFFILE(options), options->batch_,
                                    AUG_FALSE)))
        die_("aug_readservconf() failed");

    if (AUG_ISFAIL(aug_initserv()))
        die_("aug_initserv() failed");

    result = aug_runserv();
    aug_termserv();
    if (AUG_ISFAIL(result))
        die_("aug_runserv() failed");
}

static aug_bool
daemonise_(const struct aug_options* options)
{
    aug_result result = aug_daemonise(options);

    if (AUG_ISFAIL(result)) {

        if (AUG_ISNONE(result))
            return AUG_FALSE; /* Want foreground. */

        die_("aug_daemonise() failed");
    }

    return AUG_TRUE;
}

#if defined(_WIN32)
static void
start_(const struct aug_options* options)
{
    if (AUG_ISFAIL(aug_start(options)))
        die_("aug_start() failed");
}
#endif /* _WIN32 */

static void
control_(const struct aug_options* options, int sig)
{
    if (AUG_ISFAIL(aug_control(options, sig)))
        die_("aug_control() failed");
}

static void
install_(const struct aug_options* options)
{
    if (AUG_ISFAIL(aug_install(options)))
        die_("aug_install() failed");
}

static void
uninstall_(const struct aug_options* options)
{
    if (AUG_ISFAIL(aug_uninstall(options)))
        die_("aug_uninstall() failed");
}

AUGSERV_API int
aug_main(int argc, char* argv[], const struct aug_serv* serv)
{
    struct aug_options options;
    int jmpret = setjmp(mark_);
    if (jmpret)
        return jmpret;

    aug_setserv_(serv);

#if defined(_WIN32)

    /* Assume that if there is no stdin handle then the process is being
       started by the Service Control Manager. */

    if (!GetStdHandle(STD_INPUT_HANDLE)) {

        /* Note: aug_readopts() will be called from the main service
           function. */

        if (daemonise_(&options))
            return EXIT_SUCCESS;

        /* Fallthrough to run in foreground. */
    }
#endif /* _WIN32 */

    if (AUG_ISFAIL(aug_readopts(&options, argc, argv))) {
        aug_perrinfo(aug_tlx, "getreadopts() failed", NULL);
        return EXIT_FAILURE;
    }

    switch (options.command_) {
    case AUG_CMDDEFAULT:
        foreground_(&options);
        break;
    case AUG_CMDEXIT:
        return EXIT_SUCCESS;
    case AUG_CMDINSTALL:
        aug_ctxinfo(aug_tlx, "installing daemon process");
        install_(&options);
        break;
    case AUG_CMDRECONF:
        aug_ctxinfo(aug_tlx, "re-configuring daemon process");
        control_(&options, AUG_EVENTRECONF);
        break;
    case AUG_CMDSTART:
#if !defined(_WIN32)
        /* Install daemon logger prior to opening log file. */

        aug_setlog(aug_tlx, aug_getdaemonlog());
        daemonise_(&options);
#else /* _WIN32 */
        start_(&options);
#endif /* _WIN32 */
        break;
    case AUG_CMDSTATUS:
        aug_ctxinfo(aug_tlx, "getting status of daemon process");
        control_(&options, AUG_EVENTSTATUS);
        break;
    case AUG_CMDSTOP:
        aug_ctxinfo(aug_tlx, "stopping daemon process");
        control_(&options, AUG_EVENTSTOP);
        break;
    case AUG_CMDUNINSTALL:
        aug_ctxinfo(aug_tlx, "uninstalling daemon process");
        uninstall_(&options);
        break;
    }
    return EXIT_SUCCESS;
}
