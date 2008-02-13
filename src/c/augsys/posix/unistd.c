#include "augctx/errinfo.h"

#include <errno.h>
#include <unistd.h>

AUGSYS_API aug_result
aug_fclose(aug_ctx* ctx, aug_fd fd)
{
    if (-1 == close(fd)) {
        aug_setposixerrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, errno);
        return AUG_FAILURE;
    }
    return AUG_SUCCESS;
}

AUGSYS_API aug_fd
aug_vfopen(aug_ctx* ctx, const char* path, int flags, va_list args)
{
    int fd;
    mode_t mode;

    if (flags & O_CREAT)
        mode = va_arg(args, int);
    else
        mode = 0;

    if (-1 == (fd = open(path, flags, mode))) {
        aug_setposixerrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, errno);
        return -1;
    }
    return fd;
}

AUGSYS_API aug_fd
aug_fopen(aug_ctx* ctx, const char* path, int flags, ...)
{
    aug_fd fd;
    va_list args;
    va_start(args, flags);
    fd = aug_vopen(ctx, path, flags, args);
    va_end(args);
    return fd;
}

AUGSYS_API aug_result
aug_fpipe(aug_ctx* ctx, aug_fd fds[2])
{
    if (-1 == pipe(fds)) {
        aug_setposixerrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, errno);
        return AUG_FAILURE;
    }
    return AUG_SUCCESS;
}

AUGSYS_API ssize_t
aug_fread(aug_ctx* ctx, aug_fd fd, void* buf, size_t size)
{
    ssize_t ret;
    if (-1 == (ret = read(fd, buf, size))) {
        aug_setposixerrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}

AUGSYS_API ssize_t
aug_freadv(aug_ctx* ctx, aug_fd fd, const struct iovec* iov, int size)
{
    ssize_t ret;
    if (-1 == (ret = readv(fd, iov, size))) {
        aug_setposixerrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}

AUGSYS_API ssize_t
aug_fwrite(aug_ctx* ctx, aug_fd fd, const void* buf, size_t size)
{
    ssize_t ret;
    if (-1 == (ret = write(fd, buf, size))) {
        aug_setposixerrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}

AUGSYS_API ssize_t
aug_fwritev(aug_ctx* ctx, aug_fd fd, const struct iovec* iov, int size)
{
    ssize_t ret;
    if (-1 == (ret = writev(fd, iov, size))) {
        aug_setposixerrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}

AUGSYS_API aug_result
aug_fsync(aug_ctx* ctx, aug_fd fd)
{
    if (-1 == fsync(fd)) {
        aug_setposixerrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, errno);
        return AUG_FAILURE;
    }
    return AUG_SUCCESS;
}

AUGSYS_API aug_result
aug_ftruncate(aug_ctx* ctx, aug_fd fd, off_t size)
{
    if (-1 == ftruncate(fd, size)) {
        aug_setposixerrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, errno);
        return AUG_FAILURE;
    }
    return AUG_SUCCESS;
}

AUGSYS_API void
aug_msleep(aug_ctx* ctx, unsigned ms)
{
    usleep(ms * 1000);
}
