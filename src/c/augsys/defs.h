/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_DEFS_H
#define AUGSYS_DEFS_H

#include <stddef.h>

#if !defined(AUG_MAXLINE)
# define AUG_MAXLINE 1024
#endif /* !AUG_MAXLINE */

#define AUG_MIN(a, b) ((a) <= (b) ? (a) : (b))
#define AUG_MAX(a, b) ((a) >= (b) ? (a) : (b))

#define AUG_MSG(x) x

#define AUG_MKSTR_(x) #x
#define AUG_MKSTR(x) AUG_MKSTR_(x)

#define AUG_RETOK        0
#define AUG_RETERROR   (-1)
#define AUG_RETNONE    (-2)
#define AUG_RETINTR    (-3)

#if !defined(__GNUC__)
# define AUG_RCSID(x)                           \
    static const char rcsid[] = x
#else /* __GNUC__ */
# define AUG_RCSID(x)                                       \
    static const char rcsid[] __attribute__((used)) = x
#endif /* __GNUC__ */

#endif /* AUGSYS_DEFS_H */
