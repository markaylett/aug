#include "augctx/errinfo.h"

#include <errno.h>
#include <io.h>
#include <fcntl.h>

AUGSYS_API int
aug_close(aug_ctx* ctx, int fd)
{
    if (-1 == close(fd)) {
        aug_setposixerrinfo(aug_errinfo(ctx), __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUGSYS_API int
aug_vopen(aug_ctx* ctx, const char* path, int flags, va_list args)
{
    int fd;
    mode_t mode;

    if (flags & O_CREAT)
        mode = va_arg(args, int);
    else
        mode = 0;

    if (-1 == (fd = open(path, flags, mode))) {
        aug_setposixerrinfo(aug_errinfo(ctx), __FILE__, __LINE__, errno);
        return -1;
    }
    return fd;
}

AUGSYS_API int
aug_open(aug_ctx* ctx, const char* path, int flags, ...)
{
    int fd;
    va_list args;
    va_start(args, flags);
    fd = aug_vopen(path, flags, args);
    va_end(args);
    return fd;
}

AUGSYS_API int
aug_pipe(aug_ctx* ctx, int fds[2])
{
    if (-1 == pipe(fds)) {
        aug_setposixerrinfo(aug_errinfo(ctx), __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUGSYS_API ssize_t
aug_read(aug_ctx* ctx, int fd, void* buf, size_t size)
{
    ssize_t ret;
    if (-1 == (ret = read(fd, buf, size))) {
        aug_setposixerrinfo(aug_errinfo(ctx), __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}

AUGSYS_API ssize_t
aug_write(aug_ctx* ctx, int fd, const void* buf, size_t size)
{
    ssize_t ret;
    if (-1 == (ret = write(fd, buf, size))) {
        aug_setposixerrinfo(aug_errinfo(ctx), __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}

AUGSYS_API void
aug_msleep(aug_ctx* ctx, unsigned ms)
{
    usleep(ms * 1000);
}
