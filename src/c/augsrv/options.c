/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/options.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsrv/base.h"    /* aug_basename() */

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
    const char* lname = aug_getserviceopt(AUG_OPTLONGNAME);
    const char* program = aug_getserviceopt(AUG_OPTPROGRAM);
    const char* email = aug_getserviceopt(AUG_OPTEMAIL);

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
             "  test       run interactively\n"
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
        if (0 == strcasecmp(s + 1, "nstall"))
            return AUG_MKRESULT(AUG_CMDINSTALL);
        break;
    case 'r':
        if (0 == strcasecmp(s + 1, "econf"))
            return AUG_MKRESULT(AUG_CMDRECONF);
        break;
    case 's':
        if (0 == strcasecmp(s + 1, "tart"))
            return AUG_MKRESULT(AUG_CMDSTART);

        if (0 == strcasecmp(s + 1, "tatus"))
            return AUG_MKRESULT(AUG_CMDSTATUS);

        if (0 == strcasecmp(s + 1, "top"))
            return AUG_MKRESULT(AUG_CMDSTOP);
        break;
    case 't':
        if (0 == strcasecmp(s + 1, "est"))
            return AUG_MKRESULT(AUG_CMDTEST);
    case 'u':
        if (0 == strcasecmp(s + 1, "ninstall"))
            return AUG_MKRESULT(AUG_CMDUNINSTALL);
        break;
    }

    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                   AUG_MSG("invalid command '%s'"), s);
    return AUG_FAILERROR;
}

AUGSRV_API aug_result
aug_readopts(struct aug_options* options, int argc, char* argv[])
{
    int ch;
    const char* conffile;
    aug_rint rint;
    options->conffile_[0] = '\0';
    options->batch_ = 0;

    aug_optind = 1; /* Skip program name. */
    aug_opterr = 0;

    while (EOF != (ch = aug_getopt(argc, argv, "fhp")))
        switch (ch) {
        case 'b':
            options->batch_ = 1;
            break;
        case 'f':
            if (aug_optind == argc || !(conffile = argv[aug_optind++])) {
                usage_();
                aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",
                               AUG_EINVAL, AUG_MSG("missing path argument"));
                return AUG_FAILERROR;
            }
            if (!aug_realpath(options->conffile_, conffile,
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
                           AUG_EINVAL, AUG_MSG("unknown option '%c'"),
                           aug_optopt);
            return AUG_FAILERROR;
        }

    switch (argc - aug_optind) {
    case 0:
        options->command_ = AUG_CMDDEFAULT;
        break;
    case 1:
        if (AUG_ISFAIL(rint = tocommand_(argv[aug_optind]))) {
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
