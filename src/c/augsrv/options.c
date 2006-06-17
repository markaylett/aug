/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/options.h"

#include "augsrv/types.h"  /* struct aug_service */

#include "augutil/path.h"  /* aug_basename() */
#include "augutil/getopt.h"

#include "augsys/defs.h"   /* AUG_MKSTR */
#include "augsys/limits.h" /* AUG_PATH_MAX */
#include "augsys/lock.h"
#include "augsys/log.h"
#include "augsys/string.h"

#include <stdio.h>         /* EOF */
#include <string.h>

static void
usage_(const struct aug_service* service)
{
    static const char USAGE[] = "%s\n"
        "\nusage:\n"
        "  %s [options] command\n"
        "\noptions:\n"
        "  -f <conf>  specify path to configuration file\n"
        "  -h         display this usage summary and exit\n"
        "\ncommands:\n"
        "  install    install program\n"
        "  reconf     re-configure daemon\n"
        "  start      start daemon\n"
        "  status     obtain daemon's status\n"
        "  stop       stop daemon\n"
        "  test       run interactively\n"
        "  uninstall  uninstall program\n"
        "\nreport bugs to: %s\n";

    aug_info(USAGE, service->lname_, aug_basename(service->program_),
             service->admin_);
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
aug_readopts(const struct aug_service* service, int argc, char* argv[])
{
    const char* path = NULL;
    int ch, ret = AUG_CMDDEFAULT;

    aug_optind = 1; /* Skip program name. */
    aug_opterr = 0;

    while (EOF != (ch = aug_getopt(argc, argv, "fh")))
        switch (ch) {
        case 'f':
            if (aug_optind == argc
                || !*(path = argv[aug_optind++])) {

                usage_(service);
                aug_error("missing path argument");
                return -1;
            }
            break;
        case 'h':
            usage_(service);
            return AUG_CMDNONE;
        case '?':
        default:

            usage_(service);
            aug_error("unknown option '%c'", aug_optopt);
            return -1;
        }

    switch (argc - aug_optind) {
    case 0:
        break;
    case 1:
        if (-1 == (ret = aug_tocommand(argv[aug_optind]))) {

            usage_(service);
            aug_error("invalid command '%s'", argv[aug_optind]);
            return -1;
        }
        break;
    default:

        usage_(service);
        aug_error("too many arguments");
        return -1;
    }

    if (path) {

        char buf[AUG_PATH_MAX + 1];
        if (!aug_realpath(buf, path, sizeof(buf))) {
            aug_perror("aug_realpath() failed");
            return -1;
        }

        if (-1 == (*service->setopt_)(service->arg_, AUG_OPTCONFFILE, buf)) {
            aug_perror("failed to set configuration option");
            return -1;
        }
    }

    return ret;
}
