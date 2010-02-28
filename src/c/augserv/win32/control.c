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

#include "augutil/path.h"
#include "augutil/xstr.h"

#include "augsys/limits.h" /* AUG_PATH_MAX */
#include "augsys/windows.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <stdlib.h>

/* Users can define codes in the range 128 to 255. */

#define OFFSET_ 128

static aug_xstr_t
createxstr_(size_t size)
{
    aug_mpool* mpool = aug_getmpool(aug_tlx);
    aug_xstr_t s = aug_createxstr(mpool, size);
    aug_release(mpool);
    return s;
}

static aug_xstr_t
makepath_(const char* conffile)
{
    const char* program;
    char buf[AUG_PATH_MAX + 1];
    aug_xstr_t s;

    if (!(program = aug_getservopt(AUG_OPTPROGRAM))) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTPROGRAM' not set"));
        return NULL;
    }

    if (!(s = createxstr_(sizeof(buf))))
        return NULL;

    if (!aug_realpath(program, buf, sizeof(buf)))
        goto fail;

    if (aug_xstrcatc(s, '"') < 0 || aug_xstrcats(s, buf) < 0
        || aug_xstrcatc(s, '"') < 0)
        goto fail;

    if (conffile) {

        if (!aug_realpath(conffile, buf, sizeof(buf)))
            goto fail;

        if (aug_xstrcats(s, " -f \"") < 0 || aug_xstrcats(s, buf) < 0
            || aug_xstrcatc(s, '"') < 0)
            goto fail;
    }

    return s;

 fail:
    aug_destroyxstr(s);
    return NULL;
}

static aug_result
start_(SC_HANDLE scm, const struct aug_options* options)
{
    const char* sname;
    SC_HANDLE serv;
    BOOL b;
    aug_result result;
    const char* argv[] = {
        "-f", NULL
    };

    argv[1] = AUG_CONFFILE(options);

    if (!(sname = aug_getservopt(AUG_OPTSHORTNAME))) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return -1;
    }

    if (!(serv = OpenService(scm, sname, SERVICE_START))) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    /* Assume failure. */

    result = -1;

    if (argv[1]) {

        /* Specify absolute path to configuration file. */

        char buf[AUG_PATH_MAX + 1];
        if (!(argv[1] = aug_realpath(argv[1], buf, sizeof(buf))))
            goto done;

        b = StartService(serv, 2, argv);

    } else
        b = StartService(serv, 0, NULL);

    if (!b) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        goto done;
    }

    result = 0;

done:
    CloseServiceHandle(serv);
    return result;
}

static aug_result
control_(SC_HANDLE scm, int event)
{
    const char* sname;
    SC_HANDLE serv;
    SERVICE_STATUS status;
    aug_result result;

    if (!(sname = aug_getservopt(AUG_OPTSHORTNAME))) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return -1;
    }

    if (!(serv = OpenService(scm, sname, SERVICE_USER_DEFINED_CONTROL))) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    if (ControlService(serv, OFFSET_ + event, &status))
        result = 0;
    else {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        result = -1;
    }

    CloseServiceHandle(serv);
    return result;
}

static aug_result
install_(SC_HANDLE scm, const struct aug_options* options)
{
    const char* sname, * lname;
    aug_xstr_t path;
    SC_HANDLE serv;
    aug_result result;

    if (!(sname = aug_getservopt(AUG_OPTSHORTNAME))) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return -1;
    }

    if (!(lname = aug_getservopt(AUG_OPTLONGNAME))) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTLONGNAME' not set"));
        return -1;
    }

    if (!(path = makepath_(AUG_CONFFILE(options))))
        return -1;

    /* An alternative to running as "Local System" (the default), is to run as
       "NT Authority\\NetworkService". */

    if (!(serv = CreateService(scm, sname, lname, SERVICE_ALL_ACCESS,
                               SERVICE_WIN32_OWN_PROCESS,
                               SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
                               aug_xstr(path), NULL, NULL, NULL,
                               "NT Authority\\NetworkService", NULL))) {

        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        result = -1;
        goto done;
    }

    CloseServiceHandle(serv);
    result = 0;

 done:
    aug_destroyxstr(path);
    return result;
}

static aug_result
uninstall_(SC_HANDLE scm)
{
    const char* sname;
    SC_HANDLE serv;
    SERVICE_STATUS status;
    aug_result result;

    if (!(sname = aug_getservopt(AUG_OPTSHORTNAME))) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return -1;
    }

    if (!(serv = OpenService(scm, sname, SERVICE_STOP | DELETE))) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    /* Assume success. */

    result = 0;

    if (!ControlService(serv, SERVICE_CONTROL_STOP, &status)) {
        DWORD err = GetLastError();
        if (ERROR_SERVICE_NOT_ACTIVE != err) {
            aug_setwin32error(aug_tlx, __FILE__, __LINE__, err);
            result = -1;
        }
    }

    if (!DeleteService(serv) && 0 <= result) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        result = -1;
    }

    CloseServiceHandle(serv);
    return result;
}

AUGSERV_API aug_result
aug_start(const struct aug_options* options)
{
    SC_HANDLE scm;
    aug_result result;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    result = start_(scm, options);
    CloseServiceHandle(scm);
    return result;
}

AUGSERV_API aug_result
aug_control(const struct aug_options* options, int event)
{
    SC_HANDLE scm;
    aug_result result;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    result = control_(scm, event);
    CloseServiceHandle(scm);
    return result;
}

AUGSERV_API aug_result
aug_install(const struct aug_options* options)
{
    SC_HANDLE scm;
    aug_result result;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    result = install_(scm, options);
    CloseServiceHandle(scm);
    return result;
}

AUGSERV_API aug_result
aug_uninstall(const struct aug_options* options)
{
    SC_HANDLE scm;
    aug_result result;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    result = uninstall_(scm);
    CloseServiceHandle(scm);
    return result;
}
