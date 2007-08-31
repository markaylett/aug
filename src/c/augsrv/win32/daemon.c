/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrv/base.h"
#include "augsrv/options.h"

#include "augutil/event.h"
#include "augutil/path.h"   /* aug_gethome(), aug_gettmp() */

#include "augsys/errinfo.h"
#include "augsys/limits.h"
#include "augsys/log.h"
#include "augsys/utility.h" /* aug_perrinfo() */
#include "augsys/windows.h"

#include <assert.h>

/* Users can define codes in the range 128 to 255. */

#define OFFSET_  128
#define RECONF_  (OFFSET_ + AUG_EVENTRECONF)
#define STATUS_  (OFFSET_ + AUG_EVENTSTATUS)
#define STOP_    (OFFSET_ + AUG_EVENTSTOP)

static SERVICE_STATUS_HANDLE ssh_;

static int
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
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }
    return 0;
}

static void WINAPI
handler_(DWORD code)
{
    /* TODO: restore original status on failure to write event. */

    struct aug_event event = { 0, AUG_VARNULL };
    switch (code) {
    case RECONF_:
        event.type_ = AUG_EVENTRECONF;
        if (!aug_writeevent(aug_eventwr(), &event))
            aug_perrinfo(NULL, "aug_writeevent() failed");
        break;
    case STATUS_:
        event.type_ = AUG_EVENTSTATUS;
        if (!aug_writeevent(aug_eventwr(), &event))
            aug_perrinfo(NULL, "aug_writeevent() failed");
        break;
    case STOP_:
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        setstatus_(SERVICE_STOP_PENDING);
        event.type_ = AUG_EVENTSTOP;
        if (!aug_writeevent(aug_eventwr(), &event)) {
            aug_perrinfo(NULL, "aug_writeevent() failed");
            setstatus_(SERVICE_RUNNING);
        }
        break;
    }
}

static void WINAPI
start_(DWORD argc, char** argv)
{
    const char* sname;
    struct aug_options options;
    char home[AUG_PATH_MAX + 1];

    /* Fallback to tmp. */

    if (!aug_gethome(home, sizeof(home))
        && !aug_gettmp(home, sizeof(home)))
        return;

    /* Move away from system32. */

    if (!SetCurrentDirectory(home)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return;
    }

    if (!(sname = aug_getserviceopt(AUG_OPTSHORTNAME))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return;
    }

    if (-1 == aug_readopts(&options, argc, argv))
        return;

    if (AUG_CMDDEFAULT != options.command_) {

        /* Commands other than AUG_CMDDEFAULT are invalid in this context. */

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid argument(s)"));
        aug_perrinfo(NULL, "failed to read options");
        return;
    }

    if (-1 == aug_readserviceconf(*options.conffile_
                                  ? options.conffile_ : NULL, 0, 1)) {
        aug_perrinfo(NULL, "aug_readserviceconf() failed");
        return;
    }

    if (!(ssh_ = RegisterServiceCtrlHandler(sname, handler_))) {

        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        aug_perrinfo(NULL, "RegisterServiceCtrlHandler() failed");
        return;
    }

    setstatus_(SERVICE_START_PENDING);

    if (-1 == aug_initservice()) {

        aug_perrinfo(NULL, "aug_initservice() failed");
        setstatus_(SERVICE_STOPPED);
        return;
    }

    aug_notice("daemon started");
    setstatus_(SERVICE_RUNNING);

    if (-1 == aug_runservice())
        aug_perrinfo(NULL, "aug_runservice() failed");

    aug_notice("daemon stopped");
    setstatus_(SERVICE_STOPPED);

    /* This function will be called on the Service Manager's thread.  Given
       that aug_initservice() and aug_runservice() have been called on this
       thread, aug_termservice() is also called from this thread and not the
       main thread. */

    /*aug_termservice();*/
}

AUGSRV_API int
aug_daemonise(void)
{
    int ret = 0;
    const char* sname;
    SERVICE_TABLE_ENTRY table[] = {
        { NULL,  start_ },
        { NULL, NULL }
    };

    if (!(sname = aug_getserviceopt(AUG_OPTSHORTNAME))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return -1;
    }

    table[0].lpServiceName = (char*)sname;

    if (!StartServiceCtrlDispatcher(table)) {

        DWORD err = GetLastError();
        if (ERROR_FAILED_SERVICE_CONTROLLER_CONNECT == err)
            ret = AUG_RETNONE;
        else {
            aug_setwin32errinfo(NULL, __FILE__, __LINE__, err);
            ret = -1;
        }
    }

    aug_termservice();
    return ret;
}
