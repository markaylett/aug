/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrv/global.h"
#include "augsrv/options.h"
#include "augsrv/types.h" /* struct aug_service */

#include "augutil/signal.h"

#include "augsys/errno.h"
#include "augsys/log.h"
#include "augsys/string.h"
#include "augsys/windows.h"

#include <assert.h>

/* Users can define codes in the range 128 to 255. */

#define OFFSET_  128
#define RECONF_  (OFFSET_ + AUG_SIGRECONF)
#define STATUS_  (OFFSET_ + AUG_SIGSTATUS)
#define STOP_    (OFFSET_ + AUG_SIGSTOP)

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
        aug_maperror(GetLastError());
        return -1;
    }
    return 0;
}

static void WINAPI
handler_(DWORD code)
{
    /* TODO: restore original status on failure to write signal. */

    switch (code) {
    case RECONF_:
        if (-1 == aug_writesignal(aug_signalout(), AUG_SIGRECONF))
            aug_perror("failed to re-configure daemon");
        break;
    case STATUS_:
        if (-1 == aug_writesignal(aug_signalout(), AUG_SIGSTATUS))
            aug_perror("failed to get daemon status");
        break;
    case STOP_:
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        setstatus_(SERVICE_STOP_PENDING);
        if (-1 == aug_writesignal(aug_signalout(), AUG_SIGSTOP)) {
            aug_perror("failed to stop daemon");
            setstatus_(SERVICE_RUNNING);
        }
        break;
    }
}

static void WINAPI
start_(DWORD argc, char** argv)
{
    const struct aug_service* service = aug_service();
    struct aug_options options;

    if (-1 == aug_readopts(service, &options, argc, argv))
        return;

    if (AUG_CMDDEFAULT != options.command_) {

        /* Commands other than AUG_CMDDEFAULT are invalid in this context. */

        aug_error("invalid argument(s)");
        return;
    }

    if (-1 == (*service->config_)(service->arg_, options.conffile_, 1)) {
        aug_perror("failed to configure daemon");
        return;
    }

    if (!(ssh_ = RegisterServiceCtrlHandler(service->sname_, handler_))) {

        aug_maperror(GetLastError());
        aug_perror("failed to register handler");
        return;
    }

    setstatus_(SERVICE_START_PENDING);

    if (-1 == (*service->init_)(service->arg_)) {

        aug_perror("failed to initialise daemon");
        setstatus_(SERVICE_STOPPED);
        return;
    }

    aug_notice("daemon started");
    setstatus_(SERVICE_RUNNING);

    if (-1 == (*service->run_)(service->arg_))
        aug_perror("error running daemon");

    aug_notice("daemon stopped");
    setstatus_(SERVICE_STOPPED);
}

AUGSRV_API int
aug_daemonise(const struct aug_service* service)
{
    int ret = 0;
    SERVICE_TABLE_ENTRY table[] = {
        { NULL,  start_ },
        { NULL, NULL }
    };

    table[0].lpServiceName = (char*)service->sname_;

    if (!StartServiceCtrlDispatcher(table)) {

        DWORD err = GetLastError();
        if (ERROR_FAILED_SERVICE_CONTROLLER_CONNECT == err)
            ret = AUG_FOREGROUND;
        else {
            aug_maperror(err);
            ret = -1;
        }
    }
    return ret;
}
