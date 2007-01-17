/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrv/base.h"
#include "augsrv/options.h"

#include "augutil/path.h"
#include "augutil/strbuf.h"

#include "augsys/errinfo.h"
#include "augsys/limits.h" /* AUG_PATH_MAX */
#include "augsys/windows.h"

#include <stdlib.h>

/* Users can define codes in the range 128 to 255. */

#define OFFSET_ 128

static aug_strbuf_t
makepath_(void)
{
    const char* program, * conffile;
    char buf[AUG_PATH_MAX + 1];
    aug_strbuf_t s;

    if (!(program = aug_getserviceopt(AUG_OPTPROGRAM))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTPROGRAM' not set"));
        return NULL;
    }

    if (!(s = aug_createstrbuf(sizeof(buf))))
        return NULL;

    if (!aug_realpath(buf, program, sizeof(buf)))
        goto fail;

    if (-1 == aug_catstrbufc(&s, '"') || -1 == aug_catstrbufs(&s, buf)
        || -1 == aug_catstrbufc(&s, '"'))
        goto fail;

    if ((conffile = aug_getserviceopt(AUG_OPTCONFFILE))) {

        if (!aug_realpath(buf, conffile, sizeof(buf)))
            goto fail;

        if (-1 == aug_catstrbufs(&s, " -f \"")
            || -1 == aug_catstrbufs(&s, buf)
            || -1 == aug_catstrbufc(&s, '"'))
            goto fail;
    }

    return s;

 fail:
    aug_freestrbuf(s);
    return NULL;
}

static int
start_(SC_HANDLE scm)
{
    const char* sname;
    SC_HANDLE serv;
    BOOL b;
    int ret = 0;
    const char* argv[] = {
        "-f", NULL
    };

    if (!(sname = aug_getserviceopt(AUG_OPTSHORTNAME))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return -1;
    }

    argv[1] = aug_getserviceopt(AUG_OPTCONFFILE);

    if (!(serv = OpenService(scm, sname, SERVICE_START))) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
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
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        ret = -1;
    }

fail:
    CloseServiceHandle(serv);
    return ret;
}

static int
control_(SC_HANDLE scm, int event)
{
    const char* sname;
    SC_HANDLE serv;
    SERVICE_STATUS status;
    int ret = 0;

    if (!(sname = aug_getserviceopt(AUG_OPTSHORTNAME))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return -1;
    }

    if (!(serv = OpenService(scm, sname, SERVICE_USER_DEFINED_CONTROL))) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    if (!ControlService(serv, OFFSET_ + event, &status)) {

        DWORD err = GetLastError();
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, err);
        ret = -1;
    }

    CloseServiceHandle(serv);
    return ret;
}

static int
install_(SC_HANDLE scm)
{
    const char* lname, * sname;
    aug_strbuf_t path;
    SC_HANDLE serv;
    int ret = -1;

    if (!(lname = aug_getserviceopt(AUG_OPTLONGNAME))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTLONGNAME' not set"));
        return -1;
    }

    if (!(sname = aug_getserviceopt(AUG_OPTSHORTNAME))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return -1;
    }

    if (!(path = makepath_()))
        return -1;

    /* An alternative to running as "Local System" (the default), is to run as
       "NT Authority\\NetworkService". */

    if (!(serv = CreateService(scm, sname, lname, SERVICE_ALL_ACCESS,
                               SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START,
                               SERVICE_ERROR_NORMAL, aug_getstr(path), NULL,
                               NULL, NULL, "NT Authority\\NetworkService",
                               NULL))) {

        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        goto done;
    }

    CloseServiceHandle(serv);
    ret = 0;

 done:
    aug_freestrbuf(path);
    return ret;
}

static int
uninstall_(SC_HANDLE scm)
{
    const char* sname;
    SC_HANDLE serv;
    SERVICE_STATUS status;
    int ret = 0;

    if (!(sname = aug_getserviceopt(AUG_OPTSHORTNAME))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return -1;
    }

    if (!(serv = OpenService(scm, sname, SERVICE_STOP | DELETE))) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    if (!ControlService(serv, SERVICE_CONTROL_STOP, &status)) {
        DWORD err = GetLastError();
        if (ERROR_SERVICE_NOT_ACTIVE != err) {
            aug_setwin32errinfo(NULL, __FILE__, __LINE__, err);
            ret = -1;
        }
    }

    if (!DeleteService(serv) && -1 != ret) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        ret = -1;
    }

    CloseServiceHandle(serv);
    return ret;
}

AUGSRV_API int
aug_start(void)
{
    int ret;
    SC_HANDLE scm;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    ret = start_(scm);
    CloseServiceHandle(scm);
    return ret;
}

AUGSRV_API int
aug_control(int event)
{
    int ret;
    SC_HANDLE scm;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    ret = control_(scm, event);
    CloseServiceHandle(scm);
    return ret;
}

AUGSRV_API int
aug_install(void)
{
    int ret;
    SC_HANDLE scm;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    ret = install_(scm);
    CloseServiceHandle(scm);
    return ret;
}

AUGSRV_API int
aug_uninstall(void)
{
    int ret;
    SC_HANDLE scm;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    ret = uninstall_(scm);
    CloseServiceHandle(scm);
    return ret;
}
