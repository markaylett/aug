/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/base.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/errno.h"

#include <limits.h> /* INT_MAX */

#define PROCEED_ 1

static unsigned refs_ = 0;

static int
release_(void)
{
    switch (refs_) {
    case 0:
        /* Already released: do nothing. */
        break;

    case 1:
        refs_ = 0;
        return PROCEED_;

    default: /* 1 < refs_ */
        --refs_;
    }
    return 0;
}

static int
retain_(void)
{
    return 1 == ++refs_ ? PROCEED_ : 0;
}

#if !defined(_WIN32)
# include "augsys/posix/base.c"
#else /* _WIN32 */
# include "augsys/win32/base.c"
#endif /* _WIN32 */

#include "augsys/defs.h"
#include "augsys/errno.h"

#include <stdlib.h> /* atexit() */
#include <string.h> /* memcpy() */

#define BLOCKSIZE_ 256

struct file_ {
    size_t refs_;   /* number of references held. */
    const struct aug_fdtype* fdtype_;
};

static struct file_* files_ = NULL;
static size_t size_ = 0;

static void
term_(void)
{
    aug_term();
}

static void
setalreadyreg_(const char* file, int line, int fd)
{
    aug_seterrinfo(NULL, file, line, AUG_SRCLOCAL, AUG_EEXIST,
                   AUG_MSG("descriptor '%d' already registered"), (int)fd);
}

static void
setbadfd_(const char* file, int line)
{
    aug_seterrinfo(NULL, file, line, AUG_SRCLOCAL, AUG_EINVAL,
                   AUG_MSG("invalid file descriptor"));
}

static void
setnotreg_(const char* file, int line, int fd)
{
    aug_seterrinfo(NULL, file, line, AUG_SRCLOCAL, AUG_EEXIST,
                   AUG_MSG("descriptor '%d' not registered"), (int)fd);
}

static void
zerofiles_(struct file_* begin, size_t n)
{
    size_t i;
    for (i = 0; i < n; ++i) {
        begin[i].refs_ = 0;
        begin[i].fdtype_ = NULL;
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

    if (!files) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return -1;
    }

    /* Initialise new file records. */

    zerofiles_(files + size_, size - size_);

    size_ = size;
    files_ = files;
    return 0;
}

static int
openfd_(int fd, const struct aug_fdtype* fdtype)
{
    struct file_* file;

    if (size_ <= (size_t)fd) {

        if (-1 == growfiles_(fd + 1))
            return -1;

        file = files_ + fd;

    } else {

        file = files_ + fd;

        if (0 < file->refs_) {
            setalreadyreg_(__FILE__, __LINE__, fd);
            return -1;
        }
    }

    file->refs_ = 1;
    file->fdtype_ = fdtype;
    return 0;
}

static int
openfds_(int fds[2], const struct aug_fdtype* fdtype)
{
    struct file_* first, * second;
    int maxfd = AUG_MAX(fds[0], fds[1]);

    if (size_ <= (size_t)maxfd) {

        if (-1 == growfiles_(maxfd + 1))
            return -1;

        first = files_ + fds[0];
        second = files_ + fds[1];

    } else {

        first = files_ + fds[0];
        second = files_ + fds[1];

        if (0 < first->refs_) {
            setalreadyreg_(__FILE__, __LINE__, fds[0]);
            return -1;
        }

        if (0 < second->refs_) {
            setalreadyreg_(__FILE__, __LINE__, fds[1]);
            return -1;
        }
    }

    first->refs_ = 1;
    first->fdtype_ = fdtype;

    second->refs_ = 1;
    second->fdtype_ = fdtype;
    return 0;
}

static int
releasefd_(int fd, struct file_* prev)
{
    struct file_* file = files_ + fd;

    if (size_ <= (size_t)fd || files_[fd].refs_ == 0) {
        setnotreg_(__FILE__, __LINE__, fd);
        return -1;
    }

    memcpy(prev, file, sizeof(*prev));

    if (--file->refs_ == 0)
        file->fdtype_ = NULL;

    return 0;
}

static int
retainfd_(int fd)
{
    if (size_ <= (size_t)fd || files_[fd].refs_ == 0) {
        setnotreg_(__FILE__, __LINE__, fd);
        return -1;
    }

    ++files_[fd].refs_;
    return 0;
}

static const struct aug_fdtype*
setfdtype_(int fd, const struct aug_fdtype* fdtype)
{
    const struct aug_fdtype* ret;

    if (size_ <= (size_t)fd || files_[fd].refs_ == 0) {
        setnotreg_(__FILE__, __LINE__, fd);
        return NULL;
    }

    ret = files_[fd].fdtype_;
    files_[fd].fdtype_ = fdtype;
    return ret;
}

static const struct aug_fdtype*
getfdtype_(int fd)
{
    if (size_ <= (size_t)fd || 0 == files_[fd].refs_) {
        setnotreg_(__FILE__, __LINE__, fd);
        return NULL;
    }

    return files_[fd].fdtype_;
}

AUGSYS_API struct aug_errinfo*
aug_atexitinit(struct aug_errinfo* errinfo)
{
    if (!aug_init(errinfo))
        return NULL;

    if (-1 == atexit(term_)) {
        aug_term();
        return NULL;
    }

    return errinfo;
}

AUGSYS_API void
aug_exit(int status)
{
    /* If initialised, terminate now. */

    if (0 < refs_) {
        refs_ = 1;
        aug_term();
    }

    exit(status);
}

AUGSYS_API int
aug_nextid(void)
{
    static int id_ = 1;
    int id;

    aug_lock();
    if (id_ == INT_MAX) {
        id_ = 1;
        id = INT_MAX;
    } else
        id = id_++;
    aug_unlock();

    return id;
}

AUGSYS_API int
aug_openfd(int fd, const struct aug_fdtype* fdtype)
{
    int ret;

    if (-1 == fd) {
        setbadfd_(__FILE__, __LINE__);
        return -1;
    }

    if (!fdtype)
        fdtype = aug_posixfdtype();

    aug_lock();
    ret = openfd_(fd, fdtype);
    aug_unlock();

    return ret;
}

AUGSYS_API int
aug_openfds(int fds[2], const struct aug_fdtype* fdtype)
{
    int ret;

    if (-1 == fds[0] || -1 == fds[1]) {
        setbadfd_(__FILE__, __LINE__);
        return -1;
    }

    if (!fdtype)
        fdtype = aug_posixfdtype();

    aug_lock();
    ret = openfds_(fds, fdtype);
    aug_unlock();

    return ret;
}

AUGSYS_API int
aug_releasefd(int fd)
{
    struct file_ prev;

    if (-1 == fd) {
        setbadfd_(__FILE__, __LINE__);
        return -1;
    }

    aug_lock();
    if (-1 == releasefd_(fd, &prev)) {
        aug_unlock();
        return -1;
    }
    aug_unlock();

    /* The prev structure now contains the state of the file prior to the
       release operation (including ref count). */

    if (1 < prev.refs_ || !prev.fdtype_->close_)
        return  0;

    return prev.fdtype_->close_(fd);
}

AUGSYS_API int
aug_retainfd(int fd)
{
    int ret;

    if (-1 == fd) {
        setbadfd_(__FILE__, __LINE__);
        return -1;
    }

    aug_lock();
    ret = retainfd_(fd);
    aug_unlock();

    return ret;
}

AUGSYS_API const struct aug_fdtype*
aug_setfdtype(int fd, const struct aug_fdtype* fdtype)
{
    if (-1 == fd) {
        setbadfd_(__FILE__, __LINE__);
        return NULL;
    }

    if (!fdtype)
        fdtype = aug_posixfdtype();

    aug_lock();
    fdtype = setfdtype_(fd, fdtype);
    aug_unlock();

    return fdtype;
}

AUGSYS_API const struct aug_fdtype*
aug_getfdtype(int fd)
{
    const struct aug_fdtype* fdtype;

    if (-1 == fd) {
        setbadfd_(__FILE__, __LINE__);
        return NULL;
    }

    aug_lock();
    fdtype = getfdtype_(fd);
    aug_unlock();

    return fdtype;
}

AUGSYS_API struct aug_fdtype*
aug_extfdtype(struct aug_fdtype* derived, const struct aug_fdtype* base)
{
    if (!base)
        base = aug_posixfdtype();

    if (!derived->close_)
        derived->close_ = base->close_;

    if (!derived->read_)
        derived->read_ = base->read_;

    if (!derived->readv_)
        derived->readv_ = base->readv_;

    if (!derived->write_)
        derived->write_ = base->write_;

    if (!derived->writev_)
        derived->writev_ = base->writev_;

    if (!derived->setnonblock_)
        derived->setnonblock_ = base->setnonblock_;

    return derived;
}
