/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_DEFS_H
#define AUGSYS_DEFS_H

#include <stddef.h>

#define AUG_MAXLINE 1024

#define AUG_MIN(a, b) ((a) <= (b) ? (a) : (b))
#define AUG_MAX(a, b) ((a) >= (b) ? (a) : (b))

#define AUG_MKSTR_(x) #x
#define AUG_MKSTR(x) AUG_MKSTR_(x)

#define AUG_VERIFY(x, s) \
do { \
    int err = errno; \
    if (-1 == x) \
        aug_perror(s); \
    errno = err; \
} while (0)

#endif /* AUGSYS_DEFS_H */
