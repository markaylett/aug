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
#include "augserv.h"
#include "augutil.h"
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>

static const char* program_;
static char conffile_[AUG_PATH_MAX + 1] = "";
static int daemon_ = 0;

static void*
cast_(aug_task* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_taskid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
retain_(aug_task* obj)
{
}

static void
release_(aug_task* obj)
{
}

static aug_result
run_(aug_task* obj)
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

static const struct aug_taskvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    run_
};

static aug_task task_ = { &vtbl_, NULL };

static const char*
getopt_(int opt)
{
    switch (opt) {
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
readconf_(const char* conffile, aug_bool batch, aug_bool daemon)
{
    if (conffile && !aug_realpath(conffile_, conffile, sizeof(conffile_)))
        return AUG_FAILERROR;

    daemon_ = daemon;
    return AUG_SUCCESS;
}

static aug_task*
create_(void)
{
    return &task_;
}

static const struct aug_serv serv_ = {
    getopt_,
    readconf_,
    create_
};

int
main(int argc, char* argv[])
{
    program_ = argv[0];

    if (!aug_autotlx())
        return 1;

    return aug_main(argc, argv, &serv_);
}
