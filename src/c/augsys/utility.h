/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_UTILITY_H
#define AUGSYS_UTILITY_H

#include "augsys/config.h"
#include "augsys/types.h"

struct aug_errinfo;

AUGSYS_API int
aug_filesize(int fd, size_t* size);

AUGSYS_API int
aug_perrinfo(const struct aug_errinfo* errinfo, const char* s);

AUGSYS_API int
aug_perror(const char* s);

AUGSYS_API long
aug_rand(void);

AUGSYS_API void
aug_srand(unsigned seed);

AUGSYS_API int
aug_setnonblock(int fd, int on);

/**
   \return thread identifier, or 0 if the library has been compiled without
   thread support.
 */

AUGSYS_API unsigned
aug_threadid(void);

#define AUG_PERROR(x, s) \
do { \
    if (-1 == x) \
        aug_perror(s); \
} while (0)

#endif /* AUGSYS_UTILITY_H */
