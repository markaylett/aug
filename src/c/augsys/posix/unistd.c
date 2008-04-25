#include "augctx/errinfo.h"

#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>

AUGSYS_API aug_result
aug_fclose(aug_fd fd)
{
    if (-1 == close(fd)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS;
}

AUGSYS_API aug_result
aug_fsetnonblock(aug_fd fd, aug_bool on)
{
    int flags = fcntl(fd, F_GETFL);
    if (-1 == flags) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return AUG_FAILERROR;
    }

    if (on)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (-1 == fcntl(fd, F_SETFL, flags)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return AUG_FAILERROR;
    }

    return AUG_SUCCESS;
}

AUGSYS_API aug_fd
aug_vfopen(const char* path, int flags, va_list args)
{
    int fd;
    mode_t mode;

    if (flags & O_CREAT)
        mode = va_arg(args, int);
    else
        mode = 0;

    if (-1 == (fd = open(path, flags, mode))) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }
    return fd;
}

AUGSYS_API aug_fd
aug_fopen(const char* path, int flags, ...)
{
    aug_fd fd;
    va_list args;
    va_start(args, flags);
    fd = aug_vfopen(path, flags, args);
    va_end(args);
    return fd;
}

AUGSYS_API aug_result
aug_fpipe(aug_fd fds[2])
{
    if (-1 == pipe(fds)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS;
}

AUGSYS_API ssize_t
aug_fread(aug_fd fd, void* buf, size_t size)
{
    ssize_t ret;
    if (-1 == (ret = read(fd, buf, size))) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}

AUGSYS_API ssize_t
aug_fwrite(aug_fd fd, const void* buf, size_t size)
{
    ssize_t ret;
    if (-1 == (ret = write(fd, buf, size))) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}

AUGSYS_API aug_result
aug_fsync(aug_fd fd)
{
    if (-1 == fsync(fd)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS;
}

AUGSYS_API aug_result
aug_ftruncate(aug_fd fd, off_t size)
{
    if (-1 == ftruncate(fd, size)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS;
}

AUGSYS_API aug_result
aug_fsize(aug_fd fd, size_t* size)
{
    struct stat s;
    if (-1 == fstat(fd, &s)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return AUG_FAILERROR;
    }

    *size = s.st_size;
    return AUG_SUCCESS;
}

AUGSYS_API void
aug_msleep(unsigned ms)
{
    usleep(ms * 1000);
}
