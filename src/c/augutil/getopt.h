/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_GETOPT_H
#define AUGUTIL_GETOPT_H

/**
 * @file augutil/getopt.h
 * @author Henry Spencer
 *
 * Get option letter from argv.
 */

#include "augutil/config.h"

AUGUTIL_API char* aug_optarg;
AUGUTIL_API int aug_optind;
AUGUTIL_API int aug_optopt;
AUGUTIL_API int aug_opterr;

AUGUTIL_API int
aug_getopt(int argc, char** argv, const char* optstring);

#endif /* AUGUTIL_GETOPT_H */
