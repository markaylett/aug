/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>

AUGSYS_API aug_result
aug_fclose(aug_fd fd)
{
    /*
      Linus Wrote:

      If close() return EINTR, the file descriptor _will_ have been
      closed. The error return just tells you that soem error happened on the
      file: for example, in the case of EINTR, the close() may not have
      flushed all the pending data synchronously.

      Re-doing the close() is the wrong thing to do, since in a threaded
      environment, something else might have opened another file, gotten the
      same file descriptor, and you now close _another_ file.

      (Normally, re-doing the close will just return EBADF, of course).

      I'm going to drop this patch, but in case you've ever seen a case where
      EINTR actually means that the fd didn't get closed, please holler, and
      we need to fix it.
    */

    /* SYSCALL: close: EINTR */
    if (close(fd) < 0 && EINTR != errno) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUGSYS_API aug_result
aug_fsetnonblock_BI(aug_fd fd, aug_bool on)
{
    /* SYSCALL: fcntl: EAGAIN, EINTR */
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    if (on)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    /* SYSCALL: fcntl: EAGAIN, EINTR */
    if (fcntl(fd, F_SETFL, flags) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API aug_fd
aug_vfopen_N(const char* path, int flags, va_list args)
{
    int fd;
    mode_t mode;

    if (flags & O_CREAT)
        mode = va_arg(args, int);
    else
        mode = 0;

    /* SYSCALL: open: ENOENT */
    if ((fd = open(path, flags, mode)) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }
    return fd;
}

AUGSYS_API aug_fd
aug_fopen_N(const char* path, int flags, ...)
{
    aug_fd fd;
    va_list args;
    va_start(args, flags);
    fd = aug_vfopen_N(path, flags, args);
    va_end(args);
    return fd;
}

AUGSYS_API aug_result
aug_fpipe(aug_fd fds[2])
{
    if (pipe(fds) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUGSYS_API aug_rsize
aug_fread_BI(aug_fd fd, void* buf, size_t size)
{
    ssize_t ret;
    /* SYSCALL: read: EAGAIN, EINTR */
    if ((ret = read(fd, buf, size)) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}

AUGSYS_API aug_rsize
aug_fwrite_BI(aug_fd fd, const void* buf, size_t size)
{
    ssize_t ret;
    /* SYSCALL: write: EAGAIN, EINTR */
    if ((ret = write(fd, buf, size)) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}

AUGSYS_API aug_result
aug_fsync(aug_fd fd)
{
    if (fsync(fd) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUGSYS_API aug_result
aug_ftruncate_BI(aug_fd fd, off_t size)
{
    /* SYSCALL: ftruncate: EAGAIN, EINTR */
    if (ftruncate(fd, size) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUGSYS_API aug_result
aug_fsize_IN(aug_fd fd, size_t* size)
{
    struct stat s;
    /* SYSCALL: fstat: EINTR, ENOENT */
    if (fstat(fd, &s) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    *size = s.st_size;
    return 0;
}

AUGSYS_API void
aug_msleep_I(unsigned ms)
{
    /* SYSCALL: usleep: EINTR */
    usleep(ms * 1000);
}
