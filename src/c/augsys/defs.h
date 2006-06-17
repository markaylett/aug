/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_DEFS_H
#define AUGSYS_DEFS_H

#include <stddef.h>

#define AUG_MAXLINE 1024

#define AUG_MIN(a, b) ((a) <= (b) ? (a) : (b))
#define AUG_MAX(a, b) ((a) >= (b) ? (a) : (b))

#define AUG_MSG(x) x

#define AUG_MKSTR_(x) #x
#define AUG_MKSTR(x) AUG_MKSTR_(x)

#endif /* AUGSYS_DEFS_H */
