/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
#if defined(_MSC_VER)
# include <crtdbg.h>        /* _CrtSetReportMode() */
#endif /* _MSC_VER */
#include <fcntl.h>          /* _O_TEXT */
#include <io.h>             /* _open_osfhandle() */
#include <stdio.h>

/* Users can define codes in the range 128 to 255. */

#define OFFSET_  128
#define RECONF_  (OFFSET_ + AUG_EVENTRECONF)
#define STATUS_  (OFFSET_ + AUG_EVENTSTATUS)
#define STOP_    (OFFSET_ + AUG_EVENTSTOP)

static struct aug_options options_;
static SERVICE_STATUS_HANDLE ssh_;

static aug_bool
interactive_(const char* sname)
{
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    aug_bool ret = AUG_FALSE;

    if (scm) {

        SC_HANDLE serv = OpenService(scm, sname, SERVICE_QUERY_CONFIG);
        if (serv) {

            /* The MSDN example uses a buffer size of 4096. */

            union {
                QUERY_SERVICE_CONFIG config_;
                char buf_[4096];
            } u;
			DWORD dw;

            if (QueryServiceConfig(serv, &u.config_, sizeof(u.buf_), &dw)) {
                if (u.config_.dwServiceType & SERVICE_INTERACTIVE_PROCESS)
                    ret = AUG_TRUE;
            }
            CloseServiceHandle(serv);
        }
        CloseServiceHandle(scm);
    }
    return ret;
}

static aug_bool
createconsole_(const char* sname)
{
	int fd;
	FILE* fp;

	HANDLE h;
	CONSOLE_SCREEN_BUFFER_INFO info;

	if (!interactive_(sname) || !AllocConsole())
		return AUG_FALSE;

	fflush(NULL);

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
	info.dwSize.Y = 512;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), info.dwSize);

	h = GetStdHandle(STD_INPUT_HANDLE);
	fd = _open_osfhandle((intptr_t)h, _O_TEXT);
    dup2(fd, 0);
	fp = _fdopen(fd, "r");
	*stdin = *fp;
	setvbuf(stdin, NULL, _IONBF, 0);

	h = GetStdHandle(STD_OUTPUT_HANDLE);
	fd = _open_osfhandle((intptr_t)h, _O_TEXT);
    dup2(fd, 1);
	fp = _fdopen(fd, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);

	h = GetStdHandle(STD_ERROR_HANDLE);
	fd = _open_osfhandle((intptr_t)h, _O_TEXT);
    dup2(fd, 2);
	fp = _fdopen(fd, "w");
	*stderr = *fp;
	setvbuf(stderr, NULL, _IONBF, 0);

#if defined(_MSC_VER)
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif /* _MSC_VER */
	return AUG_TRUE;
}

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
    aug_result result;

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

    result = aug_writeevent(aug_events(), &event);

    if (AUG_ISFAIL(result) && !AUG_ISBLOCK(result))
        abort();
}

static void WINAPI
service_(DWORD argc, char** argv)
{
    /* Arguments are unused: these are specified as the "start parameters" in
       the Service Control Manager. */

    /* Service thread. */

    const char* sname;
    aug_bool daemon = AUG_TRUE;
    char home[AUG_PATH_MAX + 1];

	/* DebugBreak(); */

    /* Ensure writes performed on main thread are visible. */

    AUG_RMB();

    if (!aug_inittlx()) {
        fprintf(stderr, "aug_inittlx() failed\n");
        return;
    }

    if (!(sname = aug_getservopt(AUG_OPTSHORTNAME))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        aug_perrinfo(aug_tlx, "getservopt() failed", NULL);
        goto done;
    }

    /* Start console if interactive.  The console is opened before
       aug_readservconf() is called to prevent any log files from begin
       opened. */

    if (createconsole_(sname))
        daemon = AUG_FALSE;

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

    if (AUG_CMDDEFAULT != options_.command_) {

        /* Commands other than AUG_CMDDEFAULT are invalid in this context. */

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("unexpected command value"));
        aug_perrinfo(aug_tlx, "invalid options", NULL);
        goto done;
    }

    if (AUG_ISFAIL(aug_readservconf(AUG_CONFFILE(&options_), AUG_FALSE,
                                    daemon))) {
        aug_perrinfo(aug_tlx, "aug_readservconf() failed", NULL);
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
    if (!daemon)
        FreeConsole();
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

    /* Make options available to service callback. */

    memcpy(&options_, options, sizeof(options_));

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
