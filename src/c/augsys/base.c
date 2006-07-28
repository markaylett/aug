/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/base.h"
#include "augsys/errno.h"

static const char rcsid[] = "$Id:$";

#define PROCEED_ 1

static unsigned int refs_ = 0;

static int
release_(void)
{
    switch (refs_) {
    case 0:
        // Already released: do nothing.
        break;

    case 1:
        refs_ = 0;
        return PROCEED_;

    default: // 1 < refs_
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

#if defined(_MSC_VER) && !defined(_MT)
_CRTIMP int __cdecl _free_osfhnd(int);
#endif /* _MSC_VER && !_MT */

struct file_ {
    size_t refs_;
    const struct aug_fddriver* driver_;
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
    aug_seterrinfo(file, line, AUG_SRCLOCAL, AUG_EEXIST,
                   AUG_MSG("descriptor '%d' already registered"), fd);
}

static void
setbadfd_(const char* file, int line)
{
    aug_seterrinfo(file, line, AUG_SRCLOCAL, AUG_EINVAL,
                   AUG_MSG("invalid file descriptor"));
}

static void
setnotreg_(const char* file, int line, int fd)
{
    aug_seterrinfo(file, line, AUG_SRCLOCAL, AUG_EEXIST,
                   AUG_MSG("descriptor '%d' not registered"), fd);
}

static void
zerofiles_(struct file_* begin, size_t n)
{
    size_t i;
    for (i = 0; n > i; ++i) {
        begin[i].refs_ = 0;
        begin[i].driver_ = NULL;
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
        aug_setposixerrinfo(__FILE__, __LINE__, ENOMEM);
        return -1;
    }

    /* Initialise new file records. */

    zerofiles_(files + size_, size - size_);

    size_ = size;
    files_ = files;
    return 0;
}

static int
openfd_(int fd, const struct aug_fddriver* driver)
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
    file->driver_ = driver;
    return 0;
}

static int
openfds_(int fds[2], const struct aug_fddriver* driver)
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
    first->driver_ = driver;

    second->refs_ = 1;
    second->driver_ = driver;
    return 0;
}

static int
releasefd_(int fd, struct file_* current)
{
    struct file_* file = files_ + fd;

    if (size_ <= (size_t)fd || files_[fd].refs_ == 0) {
        setnotreg_(__FILE__, __LINE__, fd);
        return -1;
    }

    memcpy(current, file, sizeof(*current));

    if (--file->refs_ == 0)
        file->driver_ = NULL;

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

static int
setfddriver_(int fd, const struct aug_fddriver* driver)
{
    if (size_ <= (size_t)fd || files_[fd].refs_ == 0) {
        setnotreg_(__FILE__, __LINE__, fd);
        return -1;
    }

    files_[fd].driver_ = driver;
    return 0;
}

static const struct aug_fddriver*
fddriver_(int fd)
{
    if (size_ <= (size_t)fd || 0 == files_[fd].refs_) {
        setnotreg_(__FILE__, __LINE__, fd);
        return NULL;
    }

    return files_[fd].driver_;
}

AUGSYS_API int
aug_atexitinit(struct aug_errinfo* errinfo)
{
    if (-1 == aug_init(errinfo))
        return -1;

    if (-1 == atexit(term_)) {
        aug_term();
        return -1;
    }

    return 0;
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
aug_openfd(int fd, const struct aug_fddriver* driver)
{
    int ret;

    if (-1 == fd) {
        setbadfd_(__FILE__, __LINE__);
        return -1;
    }

    if (!driver)
        driver = aug_posixdriver();

    aug_lock();
    ret = openfd_(fd, driver);
    aug_unlock();

    return ret;
}

AUGSYS_API int
aug_openfds(int fds[2], const struct aug_fddriver* driver)
{
    int ret;

    if (-1 == fds[0] || -1 == fds[1]) {
        setbadfd_(__FILE__, __LINE__);
        return -1;
    }

    if (!driver)
        driver = aug_posixdriver();

    aug_lock();
    ret = openfds_(fds, driver);
    aug_unlock();

    return ret;
}

AUGSYS_API int
aug_releasefd(int fd)
{
    struct file_ file;

    if (-1 == fd) {
        setbadfd_(__FILE__, __LINE__);
        return -1;
    }

    aug_lock();
    if (-1 == releasefd_(fd, &file)) {
        aug_unlock();
        return -1;
    }
    aug_unlock();

    /* The file structure now contains the state of the file prior to the
       release operation (including ref count). */

    if (1 < file.refs_ || !file.driver_->close_)
        return  0;

    return file.driver_->close_(fd);
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

AUGSYS_API struct aug_fddriver*
aug_extenddriver(struct aug_fddriver* derived,
                 const struct aug_fddriver* base)
{
    if (!base)
        base = aug_posixdriver();

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

AUGSYS_API int
aug_setfddriver(int fd, const struct aug_fddriver* driver)
{
    int ret;

    if (-1 == fd) {
        setbadfd_(__FILE__, __LINE__);
        return -1;
    }

    if (!driver)
        driver = aug_posixdriver();

    aug_lock();
    ret = setfddriver_(fd, driver);
    aug_unlock();

    return ret;
}

AUGSYS_API const struct aug_fddriver*
aug_fddriver(int fd)
{
    const struct aug_fddriver* driver;

    if (-1 == fd) {
        setbadfd_(__FILE__, __LINE__);
        return NULL;
    }

    aug_lock();
    driver = fddriver_(fd);
    aug_unlock();

    return driver;
}
