/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_STRING_H
#define AUGSYS_STRING_H

#include "augsys/config.h"

#include <string.h>

/**
   All functions in this module set errno, and not errinfo.
*/

AUGSYS_API const char*
aug_strerror(int errnum);

AUGSYS_API int
aug_perror(const char* s);

AUGSYS_API size_t
aug_strlcpy(char* dst, const char* src, size_t size);

AUGSYS_API int
aug_strcasecmp(const char* lhs, const char* rhs);

AUGSYS_API int
aug_strncasecmp(const char* lhs, const char* rhs, size_t size);

#define AUG_PERROR(x, s) \
do { \
    if (-1 == x) \
        aug_perror(s); \
} while (0)

#endif /* AUGSYS_STRING_H */
