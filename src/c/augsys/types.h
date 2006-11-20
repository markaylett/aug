/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_TYPES_H
#define AUGSYS_TYPES_H

#include <stddef.h> /* size_t */
#include <sys/types.h>

#define AUG_UINT16_MAX 0xffffU
#define AUG_UINT32_MAX 0xffffffffU

#if !defined(_MSC_VER)
# include <inttypes.h>
#else /* _MSC_VER */

typedef int ssize_t;
typedef void* caddr_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned uint32_t;
typedef unsigned __int64 uint64_t;

#endif /* _MSC_VER */

typedef unsigned aug_len_t;

enum aug_loglevel {
    AUG_LOGCRIT,
    AUG_LOGERROR,
    AUG_LOGWARN,
    AUG_LOGNOTICE,
    AUG_LOGINFO,
    AUG_LOGDEBUG0
};

#endif /* AUGSYS_TYPES_H */
