/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrv/base.h"
#include "augsrv/options.h"

#include "augutil/event.h"
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
start_(DWORD argc, char** argv)
{
    /* Service thread. */

    const char* sname;
    struct aug_options options;
    char home[AUG_PATH_MAX + 1];

	/* DebugBreak(); */

    /* Ensure writes performed on main thread are visible. */

    AUG_RMB();

    if (AUG_ISFAIL(aug_initbasictlx())) {
        fprintf(stderr, "aug_initerrinfo() failed\n");
        return;
    }

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

    if (!(sname = aug_getserviceopt(AUG_OPTSHORTNAME))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        aug_perrinfo(aug_tlx, "getserviceopt() failed", NULL);
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

    if (AUG_ISFAIL(aug_readserviceconf(*options.conffile_
                                       ? options.conffile_ : NULL, 0, 1))) {
        aug_perrinfo(aug_tlx, "aug_readserviceconf() failed", NULL);
        goto done;
    }

    if (!(ssh_ = RegisterServiceCtrlHandler(sname, handler_))) {

        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        aug_perrinfo(aug_tlx, "RegisterServiceCtrlHandler() failed", NULL);
        goto done;
    }

    setstatus_(SERVICE_START_PENDING);

    if (AUG_ISFAIL(aug_initservice())) {

        aug_perrinfo(aug_tlx, "aug_initservice() failed", NULL);
        setstatus_(SERVICE_STOPPED);
        goto done;
    }

    aug_ctxnotice(aug_tlx, "daemon started");
    setstatus_(SERVICE_RUNNING);

    if (AUG_ISFAIL(aug_runservice()))
        aug_perrinfo(aug_tlx, "aug_runservice() failed", NULL);

    aug_ctxnotice(aug_tlx, "daemon stopped");
    setstatus_(SERVICE_STOPPED);

    /* This function will be called on the Service Manager's thread.  Given
       that aug_initservice() and aug_runservice() have been called on this
       thread, aug_termservice() is also called from this thread and not the
       main thread. */

    /*aug_termservice();*/

 done:

    /* Flush pending writes to main memory. */

    AUG_WMB();
    aug_term();
}

AUGSRV_API aug_result
aug_daemonise(void)
{
    const char* sname;
    SERVICE_TABLE_ENTRY table[] = {
        { NULL,  start_ },
        { NULL, NULL }
    };
    aug_result result;

    if (!(sname = aug_getserviceopt(AUG_OPTSHORTNAME))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return AUG_FAILERROR;
    }

    table[0].lpServiceName = (char*)sname;

    /* The service control dispatcher creates a new thread to execute the
       ServiceMain function of the service being started. */

    /* Flush pending writes to main memory. */

    AUG_WMB();

    if (StartServiceCtrlDispatcher(table)) {

        result = AUG_SUCCESS;

    } else {

        DWORD err = GetLastError();
        if (ERROR_FAILED_SERVICE_CONTROLLER_CONNECT == err) {
            aug_clearerrinfo(aug_tlerr);
            result = AUG_FAILNONE;
        } else
            result = aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, err);
    }

    /* Ensure writes performed on service thread are visible. */

    AUG_RMB();
    aug_termservice();
    return result;
}
