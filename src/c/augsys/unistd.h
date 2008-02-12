/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_UNISTD_H
#define AUGSYS_UNISTD_H

#include "augsys/config.h"
#include "augsys/types.h"

#include "augctx/ctx.h"
#include "augtypes.h"

AUGSYS_API aug_status
aug_close(aug_ctx* ctx, aug_fd fd);

AUGSYS_API aug_status
aug_fsync(aug_ctx* ctx, aug_fd fd);

AUGSYS_API aug_status
aug_ftruncate(aug_ctx* ctx, aug_fd fd, off_t size);

AUGSYS_API aug_fd
aug_vopen(aug_ctx* ctx, const char* path, int flags, va_list args);

AUGSYS_API aug_fd
aug_open(aug_ctx* ctx, const char* path, int flags, ...);

AUGSYS_API aug_status
aug_pipe(aug_ctx* ctx, aug_fd fds[2]);

AUGSYS_API ssize_t
aug_read(aug_ctx* ctx, aug_fd fd, void* buf, size_t size);

AUGSYS_API ssize_t
aug_write(aug_ctx* ctx, aug_fd fd, const void* buf, size_t size);

AUGSYS_API void
aug_msleep(aug_ctx* ctx, unsigned ms);

#endif /* AUGSYS_UNISTD_H */
