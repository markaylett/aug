/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_BASE_H
#define AUGSYS_BASE_H

#include "augsys/config.h"

typedef void (*aug_fdhook_t)(int, int, void*);

#define AUG_FDFILE 1
#define AUG_FDPIPE 2
#define AUG_FDSOCK 3
#define AUG_FDUSER 4

AUGSYS_API int
aug_init(void);

/* For reasons of safety, aug_term() will re-install the default logger.  The
   default logger is garaunteed to be safe even if aug_init() has not been
   called. */

AUGSYS_API int
aug_term(void);

AUGSYS_API int
aug_openfd(int fd, int type);

AUGSYS_API int
aug_releasefd(int fd);

AUGSYS_API int
aug_retainfd(int fd);

AUGSYS_API int
aug_setfdhook(int fd, aug_fdhook_t* fn, void* data);

AUGSYS_API int
aug_setfdtype(int fd, int type);

AUGSYS_API int
aug_setfddata(int fd, void* data);

AUGSYS_API int
aug_fdtype(int fd);

AUGSYS_API int
aug_fddata(int fd, void**);

#endif /* AUGSYS_BASE_H */
