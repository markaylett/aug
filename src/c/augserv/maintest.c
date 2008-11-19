/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augserv.h"
#include "augutil.h"
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>

static const char* program_;
static char conffile_[AUG_PATH_MAX + 1] = "";
static int daemon_ = 0;

static void*
cast_(aug_app* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_appid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
retain_(aug_app* obj)
{
}

static void
release_(aug_app* obj)
{
}

static const char*
getopt_(aug_app* obj, int opt)
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
readconf_(aug_app* obj, const char* conffile, aug_bool batch, aug_bool daemon)
{
    if (conffile && !aug_realpath(conffile_, conffile, sizeof(conffile_)))
        return AUG_FAILERROR;

    daemon_ = daemon;
    return AUG_SUCCESS;
}

static aug_result
init_(aug_app* obj)
{
    return AUG_SUCCESS;
}

static aug_result
run_(aug_app* obj)
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
term_(aug_app* obj)
{
}

static const struct aug_appvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    getopt_,
    readconf_,
    init_,
    run_,
    term_
};

static aug_app app_ = { &vtbl_, NULL };

int
main(int argc, char* argv[])
{
    program_ = argv[0];

    if (AUG_ISFAIL(aug_autobasictlx()))
        return 1;

    return aug_main(argc, argv, &app_);
}
