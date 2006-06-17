/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/base.h"

#if !defined(_WIN32)
# include "augsys/posix/base.c"
# include <unistd.h> /* close() */
#else /* _WIN32 */
# include "augsys/win32/base.c"
# include <io.h>     /* close() */
#endif /* _WIN32 */

#include "augsys/errno.h"

#include <stdlib.h> /* NULL */
#include <string.h> /* memcpy() */

#define BLOCKSIZE_ 256

#if defined(_WIN32) && !defined(_MT)
_CRTIMP int __cdecl _free_osfhnd(int);
#endif /* _WIN32 && !_MT */

struct file_ {
    size_t refs_;
    aug_fdhook_t fdhook_;
    int type_;
    void* data_;
};

static struct file_* files_ = NULL;
static size_t size_ = 0;

static int
close_(int fd, struct file_* file)
{
    /* The hook function should only be called once the lock has been
       released, this allows application functions to be called from the hook
       function without causing a deadlock. */

    if (file->fdhook_)
        (*file->fdhook_)(fd, file->type_, file->data_);

    if (AUG_FDUSER == file->type_)
        return 0;

#if defined(_WIN32)
    if (AUG_FDSOCK == file->type_) {

        /* The _open_osfhandle() function allocates a C run-time file handle
           and sets it to point to the operating-system file handle.  When
           _open_osfhandle() function is used on a socket descriptor, both
           _close() and closesocket() should be called before exiting.
           However, on Windows NT 4.0 Service Pack 3, closesocket() after
           _close() returns 10038. */

        closesocket(_get_osfhandle(fd));
# if defined(_MSC_VER)
        __try {
# endif /* _MSC_VER */
# if !defined(_MT)
            _free_osfhnd(fd);
# endif /* !_MT */
            close(fd);
# if defined(_MSC_VER)
        } __except (EXCEPTION_EXECUTE_HANDLER) { }
# endif /* _MSC_VER */
        return 0;
    }
#endif /* _WIN32 */
    return close(fd);
}

static void
zerofiles_(struct file_* begin, size_t n)
{
    size_t i;
    for (i = 0; n > i; ++i) {
        begin[i].refs_ = 0;
        begin[i].fdhook_ = NULL;
        begin[i].type_ = 0;
        begin[i].data_ = NULL;
    }
}

static int
growfiles_(size_t size)
{
    struct file_* files;
    size_t over = size % BLOCKSIZE_;

    /* Unless required size is an exact multiple of block size, round up to
       next block boundary. */

    if (0 != over)
        size += BLOCKSIZE_ - over;

    if (files_)
        files = realloc(files_, size * sizeof(struct file_));
    else
        files = malloc(size * sizeof(struct file_));

    if (!files)
        return -1;

    /* Initialise new file records. */

    zerofiles_(files + size_, size - size_);

    size_ = size;
    files_ = files;
    return 0;
}

static int
openfd_(int fd, int type)
{
    struct file_* file;

    if (size_ <= (size_t)fd) {

        if (-1 == growfiles_(fd + 1))
            return -1;

        file = files_ + fd;

    } else {

        file = files_ + fd;
        if (0 < file->refs_) {
            errno = EBADF;
            return -1;
        }
    }

    file->refs_ = 1;
    file->fdhook_ = NULL;
    file->type_ = type;
    file->data_ = NULL;
    return 0;
}

static int
releasefd_(int fd, struct file_* current)
{
    struct file_* file = files_ + fd;

    if (size_ <= (size_t)fd || files_[fd].refs_ == 0) {
        errno = EBADF;
        return -1;
    }

    memcpy(current, file, sizeof(*current));

    if (--file->refs_ == 0) {

        file->fdhook_ = NULL;
        file->type_ = 0;
        file->data_ = NULL;
    }
    return 0;
}

static int
retainfd_(int fd)
{
    if (size_ <= (size_t)fd || files_[fd].refs_ == 0) {
        errno = EBADF;
        return -1;
    }

    ++files_[fd].refs_;
    return 0;
}

static int
setfdhook_(int fd, aug_fdhook_t* fn, void* data)
{
    aug_fdhook_t old;
    if (size_ <= (size_t)fd || files_[fd].refs_ == 0) {
        errno = EBADF;
        return -1;
    }

    old = files_[fd].fdhook_;
    files_[fd].fdhook_ = *fn;
    files_[fd].data_ = data;
    *fn = old;
    return 0;
}

static int
setfdtype_(int fd, int type)
{
    if (size_ <= (size_t)fd || files_[fd].refs_ == 0) {
        errno = EBADF;
        return -1;
    }

    files_[fd].type_ = type;
    return 0;
}

static int
setfddata_(int fd, void* data)
{
    if (size_ <= (size_t)fd || files_[fd].refs_ == 0) {
        errno = EBADF;
        return -1;
    }

    files_[fd].data_ = data;
    return 0;
}

static int
fdtype_(int fd)
{
    if (size_ <= (size_t)fd || 0 == files_[fd].refs_) {
        errno = EBADF;
        return -1;
    }

    return files_[fd].type_;
}

static int
fddata_(int fd, void** data)
{
    if (size_ <= (size_t)fd || 0 == files_[fd].refs_) {
        errno = EBADF;
        return -1;
    }

    *data = files_[fd].data_;
    return 0;
}

AUGSYS_API int
aug_openfd(int fd, int type)
{
    int ret;

    if (-1 == fd) {
        errno = EBADF;
        return -1;
    }

    aug_lock();
    ret = openfd_(fd, type);
    aug_unlock();

    return ret;
}

AUGSYS_API int
aug_releasefd(int fd)
{
    struct file_ file;

    if (-1 == fd) {
        errno = EBADF;
        return -1;
    }

    aug_lock();
    if (-1 == releasefd_(fd, &file)) {
        aug_unlock();
        return -1;
    }
    aug_unlock();

    /* The file structure now contains the state of the file prior to the
       release operation. */

    return 1 < file.refs_ ? 0 : close_(fd, &file);
}

AUGSYS_API int
aug_retainfd(int fd)
{
    int ret;

    if (-1 == fd) {
        errno = EBADF;
        return -1;
    }

    aug_lock();
    ret = retainfd_(fd);
    aug_unlock();

    return ret;
}

AUGSYS_API int
aug_setfdhook(int fd, aug_fdhook_t* fn, void* data)
{
    int ret;

    if (-1 == fd) {
        errno = EBADF;
        return -1;
    }

    aug_lock();
    ret = setfdhook_(fd, fn, data);
    aug_unlock();

    return ret;
}

AUGSYS_API int
aug_setfdtype(int fd, int type)
{
    int ret;

    if (-1 == fd) {
        errno = EBADF;
        return -1;
    }

    aug_lock();
    ret = setfdtype_(fd, type);
    aug_unlock();

    return ret;
}

AUGSYS_API int
aug_setfddata(int fd, void* data)
{
    int ret;

    if (-1 == fd) {
        errno = EBADF;
        return -1;
    }

    aug_lock();
    ret = setfddata_(fd, data);
    aug_unlock();

    return ret;
}

AUGSYS_API int
aug_fdtype(int fd)
{
    int ret;

    if (-1 == fd) {
        errno = EBADF;
        return -1;
    }

    aug_lock();
    ret = fdtype_(fd);
    aug_unlock();

    return ret;
}

AUGSYS_API int
aug_fddata(int fd, void** data)
{
    int ret;

    if (-1 == fd) {
        errno = EBADF;
        return -1;
    }

    aug_lock();
    ret = fddata_(fd, data);
    aug_unlock();

    return ret;
}
