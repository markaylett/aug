/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_GETOPT_H
#define AUGUTIL_GETOPT_H

#include "augutil/config.h"

AUGUTIL_API char* aug_optarg;
AUGUTIL_API int aug_optind;
AUGUTIL_API int aug_optopt;
AUGUTIL_API int aug_opterr;

AUGUTIL_API int
aug_getopt(int argc, char** argv, const char* optstring);

#endif /* AUGUTIL_GETOPT_H */
