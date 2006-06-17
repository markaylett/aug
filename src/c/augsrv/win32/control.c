/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrv/options.h"

#include "augsrv/types.h"  /* struct aug_service */

#include "augutil/dstr.h"
#include "augutil/path.h"

#include "augsys/errinfo.h"
#include "augsys/limits.h" /* AUG_PATH_MAX */
#include "augsys/windows.h"

#include <stdlib.h>

/* Users can define codes in the range 128 to 255. */

#define OFFSET_ 128

static aug_dstr_t
makepath_(const struct aug_service* service)
{
    const char* program, * conffile;
    char buf[AUG_PATH_MAX + 1];
    aug_dstr_t s;

    if (!(program = service->getopt_(service->arg_, AUG_OPTPROGRAM))) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTPROGRAM' not set"));
        return NULL;
    }

    if (!(s = aug_createdstr(sizeof(buf))))
        return NULL;

    if (!aug_realpath(buf, program, sizeof(buf)))
        goto fail;

    if (-1 == aug_dstrcatc(&s, '"') || -1 == aug_dstrcats(&s, buf)
        || -1 == aug_dstrcatc(&s, '"'))
        goto fail;

    if ((conffile = service->getopt_(service->arg_, AUG_OPTCONFFILE))) {

        if (!aug_realpath(buf, conffile, sizeof(buf)))
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
start_(SC_HANDLE scm, const struct aug_service* service)
{
    const char* sname;
    SC_HANDLE serv;
    BOOL b;
    int ret = 0;
    const char* argv[] = {
        "-f", NULL
    };

    if (!(sname = service->getopt_(service->arg_, AUG_OPTSHORTNAME))) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return -1;
    }

    argv[1] = service->getopt_(service->arg_, AUG_OPTCONFFILE);

    if (!(serv = OpenService(scm, sname, SERVICE_START))) {
        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
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
        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
        ret = -1;
    }

fail:
    CloseServiceHandle(serv);
    return ret;
}

static int
control_(SC_HANDLE scm, const struct aug_service* service, aug_signal_t sig)
{
    const char* sname;
    SC_HANDLE serv;
    SERVICE_STATUS status;
    int ret = 0;

    if (!(sname = service->getopt_(service->arg_, AUG_OPTSHORTNAME))) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return -1;
    }

    if (!(serv = OpenService(scm, sname, SERVICE_USER_DEFINED_CONTROL))) {
        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
        return -1;
    }

    if (!ControlService(serv, OFFSET_ + sig, &status)) {

        DWORD err = GetLastError();
        aug_setwin32errinfo(__FILE__, __LINE__, err);
        ret = -1;
    }

    CloseServiceHandle(serv);
    return ret;
}

static int
install_(SC_HANDLE scm, const struct aug_service* service)
{
    const char* lname, * sname;
    aug_dstr_t path;
    SC_HANDLE serv;
    int ret = -1;

    if (!(lname = service->getopt_(service->arg_, AUG_OPTLONGNAME))) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTLONGNAME' not set"));
        return -1;
    }

    if (!(sname = service->getopt_(service->arg_, AUG_OPTSHORTNAME))) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return -1;
    }

    if (!(path = makepath_(service)))
        return -1;

    if (!(serv = CreateService(scm, sname, lname, SERVICE_ALL_ACCESS,
                               SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START,
                               SERVICE_ERROR_NORMAL, aug_dstr(path), NULL,
                               NULL, NULL, NULL, NULL))) {

        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
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
    const char* sname;
    SC_HANDLE serv;
    SERVICE_STATUS status;
    int ret = 0;

    if (!(sname = service->getopt_(service->arg_, AUG_OPTSHORTNAME))) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return -1;
    }

    if (!(serv = OpenService(scm, sname, SERVICE_STOP | DELETE))) {
        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
        return -1;
    }

    if (!ControlService(serv, SERVICE_CONTROL_STOP, &status)) {
        DWORD err = GetLastError();
        if (ERROR_SERVICE_NOT_ACTIVE != err) {
            aug_setwin32errinfo(__FILE__, __LINE__, err);
            ret = -1;
        }
    }

    if (!DeleteService(serv) && -1 != ret) {
        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
        ret = -1;
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
        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
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
        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
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
        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
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
        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
        return -1;
    }

    ret = uninstall_(scm, service);
    CloseServiceHandle(scm);
    return ret;
}
