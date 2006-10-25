/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/defs.h" /* AUG_MAXLINE */
#include "augsys/errinfo.h"
#include "augsys/log.h"
#include "augsys/windows.h"

#include <stdio.h>       /* _vsnprintf() */

#define AUG_MSGID 1

static aug_logger_t orig_;
static HANDLE eventlog_ = NULL;

static void
report_(WORD type, const char* format, va_list args)
{
    char msg[AUG_MAXLINE];
    const char* msgs[1];
    msgs[0] = msg;

    _vsnprintf(msg, sizeof(msg), format, args);
    msg[sizeof(msg) - 1] = '\0';

    ReportEvent(eventlog_, type, 0, AUG_MSGID, NULL, 1, 0,
                (const char**)&msgs[0], NULL);
}

static int
logger_(int loglevel, const char* format, va_list args)
{
    switch (loglevel) {
    case AUG_LOGCRIT:
    case AUG_LOGERROR:
        report_(EVENTLOG_ERROR_TYPE, format, args);
        break;
    case AUG_LOGWARN:
        report_(EVENTLOG_WARNING_TYPE, format, args);
        break;
    case AUG_LOGNOTICE:
        report_(EVENTLOG_INFORMATION_TYPE, format, args);
        break;
    }
    return (*orig_)(loglevel, format, args);
}

AUGSRV_API int
aug_setsrvlogger(const char* sname)
{
    /* Close first if already open. */

    if (eventlog_)
        aug_unsetsrvlogger();

    if (!(eventlog_ = RegisterEventSource(NULL, sname))) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    orig_ = aug_setlogger(logger_);
    return 0;
}

AUGSRV_API int
aug_unsetsrvlogger(void)
{
    if (eventlog_) {

        HANDLE eventlog = eventlog_;
        eventlog_ = NULL;

        /* Restore original logger. */

        aug_setlogger(orig_);

        if (!DeregisterEventSource(eventlog)) {
            aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
            return -1;
        }
    }
    return 0;
}
