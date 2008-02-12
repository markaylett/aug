/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_UNISTD_H
#define AUGSYS_UNISTD_H

#include "augsys/config.h"
#include "augsys/types.h"

AUGSYS_API int
aug_close(aug_ctx* ctx, int fd);

AUGSYS_API int
aug_vopen(aug_ctx* ctx, const char* path, int flags, va_list args);

AUGSYS_API int
aug_open(aug_ctx* ctx, const char* path, int flags, ...);

AUGSYS_API int
aug_pipe(aug_ctx* ctx, int fds[2]);

AUGSYS_API ssize_t
aug_read(aug_ctx* ctx, int fd, void* buf, size_t size);

AUGSYS_API ssize_t
aug_write(aug_ctx* ctx, int fd, const void* buf, size_t size);

AUGSYS_API void
aug_msleep(aug_ctx* ctx, unsigned ms);

#endif /* AUGSYS_UNISTD_H */
