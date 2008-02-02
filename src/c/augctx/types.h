/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_TYPES_H
#define AUGCTX_TYPES_H

struct timeval;

#if !defined(_WIN32)
# include <sys/uio.h>
typedef int aug_fd;
#else /* _WIN32 */
struct iovec {
    void* iov_base;
    int iov_len;
};
typedef void* aug_fd;
#endif /* _WIN32 */

enum aug_loglevel {
    AUG_LOGCRIT,
    AUG_LOGERROR,
    AUG_LOGWARN,
    AUG_LOGNOTICE,
    AUG_LOGINFO,
    AUG_LOGDEBUG0
};

struct aug_errinfo {
    char file_[512];
    int line_;
    char src_[32];
    int num_;
    char desc_[512];
};

#define AUG_SNTRUNCF(str, size, ret)            \
    do {                                        \
        (str)[(size) - 1] = '\0';               \
        if ((int)(size) <= (ret))               \
            ret = (int)(size) - 1;              \
        else if ((ret) < 0 && 0 == errno)       \
            errno = EINVAL;                     \
    } while (0)

#endif /* AUGCTX_TYPES_H */
