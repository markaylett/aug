/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augserv/base.h"
#include "augserv/options.h"
#include "augserv/types.h"

#include "augutil/event.h"
#include "augutil/log.h"
#include "augutil/path.h"   /* aug_gethome(), aug_gettmp() */

#include "augsys/barrier.h"
#include "augsys/limits.h"
#include "augsys/utility.h" /* aug_perrinfo() */
#include "augsys/windows.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augext/log.h"

#include <assert.h>
#include <stdio.h>

/* Users can define codes in the range 128 to 255. */

#define OFFSET_  128
#define RECONF_  (OFFSET_ + AUG_EVENTRECONF)
#define STATUS_  (OFFSET_ + AUG_EVENTSTATUS)
#define STOP_    (OFFSET_ + AUG_EVENTSTOP)

static SERVICE_STATUS_HANDLE ssh_;

static aug_result
setstatus_(DWORD state)
{
    SERVICE_STATUS status;

    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    status.dwCurrentState = state;
    status.dwControlsAccepted = state == SERVICE_START_PENDING
        ? 0 : SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    status.dwWin32ExitCode = 0;
    status.dwServiceSpecificExitCode = 0;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;

    if (!SetServiceStatus(ssh_, &status)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS;
}

static void WINAPI
handler_(DWORD code)
{
    /* Called on a separate thread from service_()'s, probably the main thread
       doing the StartServiceCtrlDispatcher(). */

    /* FIXME: restore original status on failure to write event. */

    struct aug_event event = { 0, NULL };
    switch (code) {
    case RECONF_:
        event.type_ = AUG_EVENTRECONF;
        break;
    case STATUS_:
        event.type_ = AUG_EVENTSTATUS;
        break;
    case STOP_:
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        setstatus_(SERVICE_STOP_PENDING);
        event.type_ = AUG_EVENTSTOP;
        break;
    }
    if (!aug_writeevent(aug_eventwr(), &event))
        abort();
}

static void WINAPI
service_(DWORD argc, char** argv)
{
    /* Service thread. */

    const char* sname;
    struct aug_options options;
    char home[AUG_PATH_MAX + 1];

	/* DebugBreak(); */

    /* Ensure writes performed on main thread are visible. */

    AUG_RMB();

    if (!aug_initdltlx()) {
        fprintf(stderr, "aug_initdltlx() failed\n");
        return;
    }

    /* Install daemon logger prior to opening log file. */

    aug_setlog(aug_tlx, aug_getdaemonlog());

    /* Fallback to tmp. */

    if (!aug_gethome(home, sizeof(home))
        && !aug_gettmp(home, sizeof(home))) {
        aug_ctxerror(aug_tlx, "failed to determine home directory");
        goto done;
    }

    /* Move away from system32. */

    if (!SetCurrentDirectory(home)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        aug_perrinfo(aug_tlx, "SetCurrentDirectory() failed", NULL);
        goto done;
    }

    if (AUG_ISFAIL(aug_readopts(&options, argc, argv))) {
        aug_perrinfo(aug_tlx, "getreadopts() failed", NULL);
        goto done;
    }

    if (AUG_CMDDEFAULT != options.command_) {

        /* Commands other than AUG_CMDDEFAULT are invalid in this context. */

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("unexpected command value"));
        aug_perrinfo(aug_tlx, "invalid options", NULL);
        goto done;
    }

    if (AUG_ISFAIL(aug_readservconf(AUG_CONFFILE(&options), AUG_FALSE,
                                    AUG_TRUE))) {
        aug_perrinfo(aug_tlx, "aug_readservconf() failed", NULL);
        goto done;
    }

    if (!(sname = aug_getservopt(AUG_OPTSHORTNAME))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        aug_perrinfo(aug_tlx, "getservopt() failed", NULL);
        goto done;
    }

    if (!(ssh_ = RegisterServiceCtrlHandler(sname, handler_))) {

        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        aug_perrinfo(aug_tlx, "RegisterServiceCtrlHandler() failed", NULL);
        goto done;
    }

    setstatus_(SERVICE_START_PENDING);

    if (AUG_ISFAIL(aug_initserv())) {

        aug_perrinfo(aug_tlx, "aug_initserv() failed", NULL);
        setstatus_(SERVICE_STOPPED);
        goto done;
    }

    aug_ctxnotice(aug_tlx, "daemon started");
    setstatus_(SERVICE_RUNNING);

    if (AUG_ISFAIL(aug_runserv()))
        aug_perrinfo(aug_tlx, "aug_runserv() failed", NULL);

    aug_ctxnotice(aug_tlx, "daemon stopped");
    setstatus_(SERVICE_STOPPED);

    /* This function will be called on the Service Manager's thread.  Given
       that aug_initserv() and aug_runserv() have been called on this thread,
       aug_termserv() is also called from this thread and not the main
       thread. */

    aug_termserv();

 done:

    /* Flush pending writes to main memory. */

    AUG_WMB();
    aug_term();
}

AUGSERV_API aug_result
aug_daemonise(const struct aug_options* options)
{
    const char* sname;
    SERVICE_TABLE_ENTRY table[] = {
        { NULL,  service_ },
        { NULL, NULL }
    };
    aug_result result;

    if (!(sname = aug_getservopt(AUG_OPTSHORTNAME))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return AUG_FAILERROR;
    }

    table[0].lpServiceName = (char*)sname;

    /* The service control dispatcher creates a new thread to execute the
       ServiceMain function of the service being started. */

    /* Flush pending writes to main memory. */

    AUG_WMB();

    /* StartServiceCtrlDispatcher() waits indefinitely for commands from the
       SCM, and returns control to the process's main function only when all
       the process' services have stopped, allowing the service process to
       clean up resources before exiting. */

    if (StartServiceCtrlDispatcher(table)) {

        result = AUG_SUCCESS;

    } else {

        DWORD err = GetLastError();
        if (ERROR_FAILED_SERVICE_CONTROLLER_CONNECT == err) {

            /* Typically, this error indicates that the program is being run
               as a console application rather than as a service.

               If the program will be run as a console application for
               debugging purposes, structure it such that service-specific
               code is not called when this error is returned. */

            aug_clearerrinfo(aug_tlerr);
            result = AUG_FAILNONE;
        } else
            result = aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, err);
    }

    /* Ensure writes performed on service thread are visible. */

    AUG_RMB();
    return result;
}
