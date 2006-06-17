/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrv.h"
#include "augutil.h"
#include "augsys.h"

#include <stdio.h>

#if defined(_MSC_VER)
# pragma comment(lib, "ws2_32.lib")
#endif /* _MSC_VER */

static const char* program_;
static char conffile_[AUG_PATH_MAX + 1] = "";
static int daemon_ = 0;

static const char*
getopt_(void* arg, enum aug_option opt)
{
    switch (opt) {
    case AUG_OPTADMIN:
        return "Mark Aylett <mark@emantic.co.uk>";
    case AUG_OPTCONFFILE:
        return *conffile_ ? conffile_ : NULL;
    case AUG_OPTLONGNAME:
        return "Test Program";
    case AUG_OPTPIDFILE:
        return "test.pid";
    case AUG_OPTPROGRAM:
        return program_;
    case AUG_OPTSHORTNAME:
        return "test";
    }
    return NULL;
}

static int
config_(void* arg, const char* conffile, int daemon)
{
    if (conffile && !aug_realpath(conffile_, conffile, sizeof(conffile_)))
        return -1;

    daemon_ = daemon;
    return 0;
}

static int
init_(void* arg)
{
    return 0;
}

static int
run_(void* arg)
{
    aug_signal_t in = 1, out = !1;

    if (-1 == aug_writesignal(aug_signalout(), in))
        return -1;

    if (-1 == aug_readsignal(aug_signalin(), &out))
        return -1;

    if (in != out) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EIO,
                       AUG_MSG("unexpected signal value from"
                               " aug_readsignal()"));
        return -1;
    }

    return 0;
}

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    struct aug_service service = {
        getopt_,
        config_,
        init_,
        run_,
        NULL
    };

    program_ = argv[0];

    aug_atexitinit(&errinfo);
    aug_main(&service, argc, argv);
    return 1;
}
