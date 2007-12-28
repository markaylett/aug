/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_TYPES_H
#define AUGSYS_TYPES_H

#include <stdarg.h>
#include <stddef.h> /* size_t */
#include <sys/types.h>

#define AUG_UINT16_MAX 0xffffU
#define AUG_UINT32_MAX 0xffffffffU

#if !defined(AUG_INTTYPES)
# define AUG_INTTYPES
# if !defined(_MSC_VER)
#  include <inttypes.h>
# else /* _MSC_VER */

typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;

typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;

typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

# endif /* _MSC_VER */
#endif /* AUG_INTTYPES */

typedef int mode_t;
typedef int pid_t;
typedef int ssize_t;
typedef void* caddr_t;

typedef unsigned aug_len_t;

#endif /* AUGSYS_TYPES_H */
