/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrv/options.h"

#include "augsrv/types.h"  /* struct aug_service */

#include "augutil/dstr.h"
#include "augutil/path.h"

#include "augsys/errno.h"
#include "augsys/limits.h" /* AUG_PATH_MAX */
#include "augsys/windows.h"

#include <stdlib.h>

/* Users can define codes in the range 128 to 255. */

#define OFFSET_ 128

static aug_dstr_t
makepath_(const struct aug_service* service)
{
    char buf[AUG_PATH_MAX + 1];
    aug_dstr_t s;
    const char* path;

    if (!(s = aug_createdstr(sizeof(buf))))
        return NULL;

    if (!aug_realpath(buf, service->program_, sizeof(buf)))
        goto fail;

    if (-1 == aug_dstrcatc(&s, '"') || -1 == aug_dstrcats(&s, buf)
        || -1 == aug_dstrcatc(&s, '"'))
        goto fail;

    if ((path = (*service->getopt_)(service->arg_, AUG_OPTCONFFILE))) {

        if (!aug_realpath(buf, path, sizeof(buf)))
            goto fail;

        if (-1 == aug_dstrcats(&s, " -f \"") || -1 == aug_dstrcats(&s, buf)
            || -1 == aug_dstrcatc(&s, '"'))
            goto fail;
    }

    return s;

 fail:
    aug_freedstr(s);
    return NULL;
}

static int
installed_(SC_HANDLE scm, const struct aug_service* service)
{
    SC_HANDLE serv = OpenService(scm, service->sname_, SERVICE_QUERY_CONFIG);
    if (!serv)
        return 0;

    CloseServiceHandle(serv);
    return 1;
}

static int
start_(SC_HANDLE scm, const struct aug_service* service)
{
    SC_HANDLE serv;
    const char* argv[] = {
        "-f", (*service->getopt_)(service->arg_, AUG_OPTCONFFILE)
    };
    BOOL b;
    int ret = 0;

    if (!(serv = OpenService(scm, service->sname_, SERVICE_START))) {

        DWORD err = GetLastError();
        if (ERROR_SERVICE_DOES_NOT_EXIST == err)
            return AUG_ENOTEXISTS;

        aug_maperror(err);
        return -1;
    }

    if (argv[1]) {

        /* Specify absolute path to configuration file. */

        char buf[AUG_PATH_MAX + 1];
        if (!(argv[1] = aug_realpath(buf, argv[1], sizeof(buf)))) {
            ret = -1;
            goto fail;
        }

        b = StartService(serv, 2, argv);

    } else
        b = StartService(serv, 0, NULL);

    if (!b) {

        DWORD err = GetLastError();
        if (ERROR_SERVICE_ALREADY_RUNNING == err)
            ret = AUG_EEXISTS;
        else {
            aug_maperror(err);
            ret = -1;
        }
    }

fail:
    CloseServiceHandle(serv);
    return ret;
}

static int
control_(SC_HANDLE scm, const struct aug_service* service, aug_signal_t sig)
{
    SC_HANDLE serv;
    SERVICE_STATUS status;
    int ret = 0;

    if (!(serv = OpenService(scm, service->sname_,
                             SERVICE_USER_DEFINED_CONTROL))) {
        DWORD err = GetLastError();
        if (ERROR_SERVICE_DOES_NOT_EXIST == err)
            return AUG_ENOTEXISTS;

        aug_maperror(err);
        return -1;
    }

    if (!ControlService(serv, OFFSET_ + sig, &status)) {

        DWORD err = GetLastError();
        if (ERROR_SERVICE_NOT_ACTIVE == err)
            ret = AUG_ENOTEXISTS;
        else if (ERROR_SERVICE_CANNOT_ACCEPT_CTRL == err) {
            errno = EBUSY;
            ret = -1;
        } else {
            aug_maperror(err);
            ret = -1;
        }
    }

    CloseServiceHandle(serv);
    return ret;
}

static int
install_(SC_HANDLE scm, const struct aug_service* service)
{
    aug_dstr_t path;
    SC_HANDLE serv;
    int ret = -1;

    if (installed_(scm, service))
        return AUG_EEXISTS;

    if (!(path = makepath_(service)))
        return -1;

    if (!(serv = CreateService(scm, service->sname_, service->lname_,
                               SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                               SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
                               aug_dstr(path), NULL, NULL, NULL, NULL,
                               NULL))) {

        aug_maperror(GetLastError());
        goto done;
    }

    CloseServiceHandle(serv);
    ret = 0;

 done:
    aug_freedstr(path);
    return ret;
}

static int
uninstall_(SC_HANDLE scm, const struct aug_service* service)
{
    SC_HANDLE serv;
    SERVICE_STATUS status;
    int ret = 0;

    if (!installed_(scm, service))
        return AUG_ENOTEXISTS;

    if (!(serv = OpenService(scm, service->sname_, SERVICE_STOP | DELETE))) {
        DWORD err = GetLastError();
        if (ERROR_SERVICE_DOES_NOT_EXIST == err)
            return AUG_ENOTEXISTS;

        aug_maperror(err);
        return -1;
    }

    if (!ControlService(serv, SERVICE_CONTROL_STOP, &status)) {

        DWORD err = GetLastError();
        if (ERROR_SERVICE_NOT_ACTIVE != err) {
            aug_maperror(err);
            ret = -1;
        }
    }

    if (!DeleteService(serv) && -1 != ret) {

        DWORD err = GetLastError();
        if (ERROR_SERVICE_MARKED_FOR_DELETE != err) {
            aug_maperror(err);
            ret = -1;
        }
    }

    CloseServiceHandle(serv);
    return ret;
}

AUGSRV_API int
aug_start(const struct aug_service* service)
{
    int ret;
    SC_HANDLE scm;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        aug_maperror(GetLastError());
        return -1;
    }

    ret = start_(scm, service);
    CloseServiceHandle(scm);
    return ret;
}

AUGSRV_API int
aug_control(const struct aug_service* service, aug_signal_t sig)
{
    int ret;
    SC_HANDLE scm;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        aug_maperror(GetLastError());
        return -1;
    }

    ret = control_(scm, service, sig);
    CloseServiceHandle(scm);
    return ret;
}

AUGSRV_API int
aug_install(const struct aug_service* service)
{
    int ret;
    SC_HANDLE scm;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        aug_maperror(GetLastError());
        return -1;
    }

    ret = install_(scm, service);
    CloseServiceHandle(scm);
    return ret;
}

AUGSRV_API int
aug_uninstall(const struct aug_service* service)
{
    int ret;
    SC_HANDLE scm;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        aug_maperror(GetLastError());
        return -1;
    }

    ret = uninstall_(scm, service);
    CloseServiceHandle(scm);
    return ret;
}
