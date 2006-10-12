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

#define AUG_RETSUCCESS 0
#define AUG_RETERROR   (-1)
#define AUG_RETNONE    (-2)
#define AUG_RETINTR    (-3)

#endif /* AUGSYS_DEFS_H */
