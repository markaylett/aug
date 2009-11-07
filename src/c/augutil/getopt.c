/*
 * getopt - get option letter from argv
 *
 * This is a version of the public domain getopt() implementation by
 * Henry Spencer, changed for 4.3BSD compatibility (in addition to System V).
 * It allows rescanning of an option list by setting optind to 0 before
 * calling, which is why we use it even if the system has its own (in fact,
 * this one has a unique name so as not to conflict with the system's).
 * Thanks to Dennis Ferguson for the appropriate modifications.
 *
 * This file is in the Public Domain.
 */

/*LINTLIBRARY*/

#define AUGUTIL_BUILD
#include "augutil/getopt.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include <stdio.h>

#ifdef    lint
#undef    putc
#define    putc    fputc
#endif    /* lint */

static char    *optarg;    /* Global argument pointer. */
static int    optind = 0;    /* Global argv index. */
static int    opterr = 1;    /* for compatibility, should error be printed? */
static int    optopt;    /* for compatibility, option character checked */

static char    *scan = NULL;    /* Private scan pointer. */
static const char    *prog = "amnesia";

/*
 * Print message about a bad option.
 */
static int
badopt(
       const char *mess,
       int ch
       )
{
    if (opterr) {
        fputs(prog, stderr);
        fputs(mess, stderr);
        (void) putc(ch, stderr);
        (void) putc('\n', stderr);
    }
    return ('?');
}

AUGUTIL_API char*
aug_optarg_(void)
{
    return optarg;
}

AUGUTIL_API int*
aug_optind_(void)
{
    return &optind;
}

AUGUTIL_API int*
aug_optopt_(void)
{
    return &optopt;
}

AUGUTIL_API int*
aug_opterr_(void)
{
    return &opterr;
}

AUGUTIL_API int
aug_getopt(
           int argc,
           char *argv[],
           const char *optstring
           )
{
    register char c;
    register const char *place;

    prog = argv[0];
    optarg = NULL;

    if (optind == 0) {
        scan = NULL;
        optind++;
    }

    if (scan == NULL || *scan == '\0') {
        if (optind >= argc
            || argv[optind][0] != '-'
            || argv[optind][1] == '\0') {
            return (EOF);
        }
        if (argv[optind][1] == '-'
            && argv[optind][2] == '\0') {
            optind++;
            return (EOF);
        }

        scan = argv[optind++]+1;
    }

    c = *scan++;
    optopt = c & 0377;
    for (place = optstring; place != NULL && *place != '\0'; ++place)
        if (*place == c)
            break;

    if (place == NULL || *place == '\0' || c == ':' || c == '?') {
        return (badopt(": unknown option -", c));
    }

    place++;
    if (*place == ':') {
        if (*scan != '\0') {
            optarg = scan;
            scan = NULL;
        } else if (optind >= argc) {
            return (badopt(": option requires argument -", c));
        } else {
            optarg = argv[optind++];
        }
    }

    return (c & 0377);
}
