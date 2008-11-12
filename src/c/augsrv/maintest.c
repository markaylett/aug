/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augsrv.h"
#include "augutil.h"
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>

static const char* program_;
static char conffile_[AUG_PATH_MAX + 1] = "";
static int daemon_ = 0;

static const char*
getopt_(void* arg, enum aug_option opt)
{
    switch (opt) {
    case AUG_OPTCONFFILE:
        return *conffile_ ? conffile_ : NULL;
    case AUG_OPTEMAIL:
        return "Mark Aylett <mark.aylett@gmail.com>";
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

static aug_result
config_(void* arg, const char* conffile, int batch, int daemon)
{
    if (conffile && !aug_realpath(conffile_, conffile, sizeof(conffile_)))
        return AUG_FAILERROR;

    daemon_ = daemon;
    return AUG_SUCCESS;
}

static aug_result
init_(void* arg)
{
    return AUG_SUCCESS;
}

static aug_result
run_(void* arg)
{
    struct aug_event in = { 1, 0 }, out = { !1, 0 };

    /* Sticky events not required for fixed length blocking read. */

    if (!aug_writeevent(aug_eventwr(), &in))
        return AUG_FAILERROR;

    if (!aug_readevent(aug_eventrd(), &out))
        return AUG_FAILERROR;

    if (in.type_ != out.type_) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EIO,
                       AUG_MSG("unexpected event type from aug_readevent()"));
        return AUG_FAILERROR;
    }

    return AUG_SUCCESS;
}

static void
term_(void* arg)
{
}

int
main(int argc, char* argv[])
{
    struct aug_service service = {
        getopt_,
        config_,
        init_,
        run_,
        term_
    };

    program_ = argv[0];

    if (AUG_ISFAIL(aug_autobasictlx()))
        return 1;

    return aug_main(argc, argv, &service, NULL);
}
