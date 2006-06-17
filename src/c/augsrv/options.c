/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/options.h"

static const char rcsid[] = "$Id:$";

#include "augsrv/types.h"  /* struct aug_service */

#include "augutil/path.h"  /* aug_basename() */
#include "augutil/getopt.h"

#include "augsys/defs.h"   /* AUG_MKSTR */
#include "augsys/log.h"
#include "augsys/string.h"

#include <stdio.h>         /* EOF */
#include <string.h>

static void
usage_(const struct aug_service* service)
{
    const char* lname = service->getopt_(service->arg_, AUG_OPTLONGNAME);
    const char* program = service->getopt_(service->arg_, AUG_OPTPROGRAM);
    const char* admin = service->getopt_(service->arg_, AUG_OPTADMIN);

    if (lname)
        aug_info("%s\n", lname);

    if (program)
        aug_info("usage:\n"
                 "  %s [options] command\n", aug_basename(program));

    aug_info("options:\n"
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

    if (admin)
        aug_info("report bugs to: %s\n", admin);
}

AUGSRV_API int
aug_tocommand(const char* s)
{
    if (!s)
        return AUG_CMDDEFAULT;

    switch (*s) {
    case 'i':
        if (0 == strcmp(s + 1, "nstall"))
            return AUG_CMDINSTALL;
        break;
    case 'r':
        if (0 == strcmp(s + 1, "econf"))
            return AUG_CMDRECONF;
        break;
    case 's':
        if (0 == strcmp(s + 1, "tart"))
            return AUG_CMDSTART;

        if (0 == strcmp(s + 1, "tatus"))
            return AUG_CMDSTATUS;

        if (0 == strcmp(s + 1, "top"))
            return AUG_CMDSTOP;
        break;
    case 't':
        if (0 == strcmp(s + 1, "est"))
            return AUG_CMDTEST;
    case 'u':
        if (0 == strcmp(s + 1, "ninstall"))
            return AUG_CMDUNINSTALL;
        break;
    }
    return -1;
}

AUGSRV_API int
aug_readopts(const struct aug_service* service, struct aug_options* options,
             int argc, char* argv[])
{
    int ch, ret;
    options->conffile_ = NULL;

    aug_optind = 1; /* Skip program name. */
    aug_opterr = 0;

    while (EOF != (ch = aug_getopt(argc, argv, "fh")))
        switch (ch) {
        case 'f':
            if (aug_optind == argc
                || !*(options->conffile_ = argv[aug_optind++])) {

                usage_(service);
                aug_error("missing path argument");
                return -1;
            }
            break;
        case 'h':
            options->command_ = AUG_CMDEXIT;
            usage_(service);
            return 0;
        case '?':
        default:

            usage_(service);
            aug_error("unknown option '%c'", aug_optopt);
            return -1;
        }

    switch (argc - aug_optind) {
    case 0:
        options->command_ = AUG_CMDDEFAULT;
        break;
    case 1:
        if (-1 == (ret = aug_tocommand(argv[aug_optind]))) {

            usage_(service);
            aug_error("invalid command '%s'", argv[aug_optind]);
            return -1;
        }
        options->command_ = ret;
        break;
    default:

        usage_(service);
        aug_error("too many arguments");
        return -1;
    }

    return 0;
}
