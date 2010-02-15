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
#define AUGSERV_BUILD
#include "augserv/options.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augserv/base.h"   /* aug_basename() */
#include "augserv/types.h"

#include "augutil/path.h"   /* aug_basename() */
#include "augutil/getopt.h"

#include "augctx/defs.h"    /* AUG_MKSTR */
#include "augsys/utility.h" /* aug_perrinfo() */

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/string.h"

#include <stdio.h>          /* EOF */
#include <string.h>

static void
usage_(void)
{
    const char* lname = aug_getservopt(AUG_OPTLONGNAME);
    const char* program = aug_getservopt(AUG_OPTPROGRAM);
    const char* email = aug_getservopt(AUG_OPTEMAIL);

    if (lname)
        aug_ctxinfo(aug_tlx, "%s\n", lname);

    if (program)
        aug_ctxinfo(aug_tlx, "usage:\n"
                    "  %s [options] command\n", aug_basename(program));

    aug_ctxinfo(aug_tlx, "options:\n"
             "  -b         batch mode - no interactive prompts\n"
             "  -f <conf>  specify path to configuration file\n"
             "  -h         display this usage summary and exit\n"
             "\ncommands:\n"
             "  install    install program\n"
             "  reconf     re-configure daemon\n"
             "  start      start daemon\n"
             "  status     obtain daemon's status\n"
             "  stop       stop daemon\n"
             "  uninstall  uninstall program\n");

    if (email)
        aug_ctxinfo(aug_tlx, "report bugs to: %s\n", email);
}

static aug_rint
tocommand_(const char* s)
{
    if (!s)
        return AUG_MKRESULT(AUG_CMDDEFAULT);

    switch (*s) {
    case 'i':
        if (0 == aug_strcasecmp(s + 1, "nstall"))
            return AUG_MKRESULT(AUG_CMDINSTALL);
        break;
    case 'r':
        if (0 == aug_strcasecmp(s + 1, "econf"))
            return AUG_MKRESULT(AUG_CMDRECONF);
        break;
    case 's':
        if (0 == aug_strcasecmp(s + 1, "tart"))
            return AUG_MKRESULT(AUG_CMDSTART);

        if (0 == aug_strcasecmp(s + 1, "tatus"))
            return AUG_MKRESULT(AUG_CMDSTATUS);

        if (0 == aug_strcasecmp(s + 1, "top"))
            return AUG_MKRESULT(AUG_CMDSTOP);
        break;
    case 'u':
        if (0 == aug_strcasecmp(s + 1, "ninstall"))
            return AUG_MKRESULT(AUG_CMDUNINSTALL);
        break;
    }

    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                   AUG_MSG("invalid command [%s]"), s);
    return AUG_FAILERROR;
}

AUGSERV_API aug_result
aug_readopts(struct aug_options* options, int argc, char* argv[])
{
    int ch;
    const char* conffile;
    aug_rint rint;

    /* Defaults. */

    options->conffile_[0] = '\0';
    options->batch_ = AUG_FALSE;

    aug_optind = 1; /* Skip program name. */
    aug_opterr = 0;

    while (EOF != (ch = aug_getopt(argc, argv, "fhp")))
        switch (ch) {
        case 'b':
            options->batch_ = AUG_TRUE;
            break;
        case 'f':
            if (aug_optind == argc || !(conffile = argv[aug_optind++])) {
                usage_();
                aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",
                               AUG_EINVAL, AUG_MSG("missing path argument"));
                return AUG_FAILERROR;
            }
            if (!aug_realpath(conffile, options->conffile_,
                              sizeof(options->conffile_))) {
                usage_();
                return AUG_FAILERROR;
            }
            break;
        case 'h':
            options->command_ = AUG_CMDEXIT;
            usage_();
            return AUG_SUCCESS;
        case '?':
        default:
            usage_();
            aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",
                           AUG_EINVAL, AUG_MSG("unknown option [%c]"),
                           aug_optopt);
            return AUG_FAILERROR;
        }

    switch (argc - aug_optind) {
    case 0:
        options->command_ = AUG_CMDDEFAULT;
        break;
    case 1:
        if (aug_isfail(rint = tocommand_(argv[aug_optind]))) {
            usage_();
            return rint;
        }
        options->command_ = AUG_RESULT(rint);
        break;
    default:
        usage_();
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",
                       AUG_EINVAL, AUG_MSG("too many arguments"));
        return AUG_FAILERROR;
    }

    return AUG_SUCCESS;
}
