/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
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
makepath_(void)
{
    const char* program, * conffile;
    char buf[AUG_PATH_MAX + 1];
    aug_xstr_t s;

    if (!(program = aug_getservopt(AUG_OPTPROGRAM))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTPROGRAM' not set"));
        return NULL;
    }

    if (!(s = createxstr_(sizeof(buf))))
        return NULL;

    if (!aug_realpath(buf, program, sizeof(buf)))
        goto fail;

    if (AUG_ISFAIL(aug_xstrcatc(s, '"')) || AUG_ISFAIL(aug_xstrcats(s, buf))
        || AUG_ISFAIL(aug_xstrcatc(s, '"')))
        goto fail;

    if ((conffile = aug_getservopt(AUG_OPTCONFFILE))) {

        if (!aug_realpath(buf, conffile, sizeof(buf)))
            goto fail;

        if (AUG_ISFAIL(aug_xstrcats(s, " -f \""))
            || AUG_ISFAIL(aug_xstrcats(s, buf))
            || AUG_ISFAIL(aug_xstrcatc(s, '"')))
            goto fail;
    }

    return s;

 fail:
    aug_destroyxstr(s);
    return NULL;
}

static aug_result
start_(SC_HANDLE scm)
{
    const char* sname;
    SC_HANDLE serv;
    BOOL b;
    aug_result result;
    const char* argv[] = {
        "-f", NULL
    };

    if (!(sname = aug_getservopt(AUG_OPTSHORTNAME))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return AUG_FAILERROR;
    }

    argv[1] = aug_getservopt(AUG_OPTCONFFILE);

    if (!(serv = OpenService(scm, sname, SERVICE_START)))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    if (argv[1]) {

        /* Specify absolute path to configuration file. */

        char buf[AUG_PATH_MAX + 1];
        if (!(argv[1] = aug_realpath(buf, argv[1], sizeof(buf)))) {
            result = AUG_FAILERROR;
            goto done;
        }

        b = StartService(serv, 2, argv);

    } else
        b = StartService(serv, 0, NULL);

    result = b ? AUG_SUCCESS
        : aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());

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
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return AUG_FAILERROR;
    }

    if (!(serv = OpenService(scm, sname, SERVICE_USER_DEFINED_CONTROL)))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    if (ControlService(serv, OFFSET_ + event, &status))
        result = AUG_SUCCESS;
    else
        result = aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                     GetLastError());

    CloseServiceHandle(serv);
    return result;
}

static aug_result
install_(SC_HANDLE scm)
{
    const char* lname, * sname;
    aug_xstr_t path;
    SC_HANDLE serv;
    aug_result result;

    if (!(lname = aug_getservopt(AUG_OPTLONGNAME))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTLONGNAME' not set"));
        return AUG_FAILERROR;
    }

    if (!(sname = aug_getservopt(AUG_OPTSHORTNAME))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return AUG_FAILERROR;
    }

    if (!(path = makepath_()))
        return AUG_FAILERROR;

    /* An alternative to running as "Local System" (the default), is to run as
       "NT Authority\\NetworkService". */

    if (!(serv = CreateService(scm, sname, lname, SERVICE_ALL_ACCESS,
                               SERVICE_WIN32_OWN_PROCESS,
                               SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
                               aug_xstr(path), NULL, NULL, NULL,
                               "NT Authority\\NetworkService", NULL))) {

        result = aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                     GetLastError());
        goto done;
    }

    CloseServiceHandle(serv);
    result = AUG_SUCCESS;

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
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTSHORTNAME' not set"));
        return AUG_FAILERROR;
    }

    if (!(serv = OpenService(scm, sname, SERVICE_STOP | DELETE)))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    /* Assume success. */

    result = AUG_SUCCESS;

    if (!ControlService(serv, SERVICE_CONTROL_STOP, &status)) {
        DWORD err = GetLastError();
        if (ERROR_SERVICE_NOT_ACTIVE != err)
            result = aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, err);
    }

    if (!DeleteService(serv) && AUG_ISSUCCESS(result))
        result = aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                     GetLastError());

    CloseServiceHandle(serv);
    return result;
}

AUGSERV_API aug_result
aug_start(void)
{
    SC_HANDLE scm;
    aug_result result;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    result = start_(scm);
    CloseServiceHandle(scm);
    return result;
}

AUGSERV_API aug_result
aug_control(int event)
{
    SC_HANDLE scm;
    aug_result result;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    result = control_(scm, event);
    CloseServiceHandle(scm);
    return result;
}

AUGSERV_API aug_result
aug_install(void)
{
    SC_HANDLE scm;
    aug_result result;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    result = install_(scm);
    CloseServiceHandle(scm);
    return result;
}

AUGSERV_API aug_result
aug_uninstall(void)
{
    SC_HANDLE scm;
    aug_result result;

    if (!(scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    result = uninstall_(scm);
    CloseServiceHandle(scm);
    return result;
}
