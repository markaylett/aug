/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_TYPES_H
#define AUGSYS_TYPES_H

#include <stddef.h> /* size_t */
#include <sys/types.h>

#if defined(_WIN32)
# if defined(_MSC_VER)
typedef int ssize_t;
typedef void* caddr_t;
# endif /* _MSC_VER */
#endif /* _WIN32 */

enum aug_loglevel {
    AUG_LOGCRIT,
    AUG_LOGERROR,
    AUG_LOGWARN,
    AUG_LOGNOTICE,
    AUG_LOGINFO,
    AUG_LOGDEBUG
};

#endif /* AUGSYS_TYPES_H */
