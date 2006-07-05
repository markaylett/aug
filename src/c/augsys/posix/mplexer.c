/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/defs.h"   /* AUG_MAX */
#include "augsys/errinfo.h"
#include "augsys/time.h"   /* aug_tvtoms() */
#include "augsys/unistd.h" /* aug_pipe() */

#include <errno.h>
#include <stdlib.h>        /* malloc() */

#include <sys/poll.h>

#define INIT_SIZE_ 64

struct aug_mplexer_ {
    struct pollfd* pollfds_;
    size_t nfds_, size_;
};

static unsigned short
external_(short src)
{
    unsigned short dst = 0;

    if (src & (POLLHUP | POLLIN))
        dst |= AUG_IOEVENTRD;

    if (src & POLLOUT)
        dst |= AUG_IOEVENTWR;

    return dst;
}

static short
internal_(unsigned short src)
{
    short dst = 0;

    if (src & AUG_IOEVENTRD)
        dst |= (POLLHUP | POLLIN);

    if (src & AUG_IOEVENTWR)
        dst |= POLLOUT;

    return dst;
}

static void
initpollfd_(struct pollfd* p)
{
    p->fd = -1;
    p->events = p->revents = 0;
}

static void
initpollfds_(struct pollfd* ptr, size_t size)
{
    size_t i = 0;
    for (; i < size; ++i)
        initpollfd_(ptr + i);
}

static int
resize_(aug_mplexer_t mplexer, size_t size)
{
    struct pollfd* ptr = realloc(mplexer->pollfds_,
                                 sizeof(struct pollfd) * size);
    if (!ptr) {
        aug_setposixerrinfo(__FILE__, __LINE__, ENOMEM);
        return -1;
    }

    initpollfds_(ptr + mplexer->size_, size - mplexer->size_);
    mplexer->pollfds_ = ptr;
    mplexer->size_ = size;
    return 0;
}

AUGSYS_API aug_mplexer_t
aug_createmplexer(void)
{
    aug_mplexer_t mplexer = malloc(sizeof(struct aug_mplexer_));
    if (!mplexer) {
        aug_setposixerrinfo(__FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    mplexer->pollfds_ = NULL;
    mplexer->nfds_ = mplexer->size_ = 0;

    if (-1 == resize_(mplexer, INIT_SIZE_)) {
        free(mplexer);
        return NULL;
    }

    return mplexer;
}

AUGSYS_API int
aug_freemplexer(aug_mplexer_t mplexer)
{
    free(mplexer->pollfds_);
    free(mplexer);
    return 0;
}

AUGSYS_API int
aug_setioeventmask(aug_mplexer_t mplexer, int fd, unsigned short mask)
{
    struct pollfd* ptr;

    if (mask & ~(AUG_IOEVENTRD | AUG_IOEVENTWR)) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid ioevent mask '%d'"), mask);
        return -1;
    }

    if (mplexer->size_ <= fd)
        if (-1 == resize_(mplexer, AUG_MAX(fd + 1, mplexer->size_ * 2)))
            return -1;

    ptr = mplexer->pollfds_ + fd;
    if (mask) {

        ptr->fd = fd;
        ptr->events = internal_(mask);

        if (mplexer->nfds_ <= fd)
            mplexer->nfds_ = fd + 1;

    } else {

        initpollfd_(ptr);

        if (mplexer->nfds_ == fd + 1) {

            for (--fd; fd >= 0 && -1 == mplexer->pollfds_[fd].fd; --fd)
                ;
            mplexer->nfds_ = fd + 1;
        }
    }
    return 0;
}

AUGSYS_API int
aug_waitioevents(aug_mplexer_t mplexer, const struct timeval* timeout)
{
    int ms, ret;

    ms = timeout ? aug_tvtoms(timeout) : -1;

    if (-1 == (ret = poll(mplexer->pollfds_, mplexer->nfds_, ms))) {

        if (EINTR == aug_setposixerrinfo(__FILE__, __LINE__, errno))
            ret = AUG_RETINTR;
    }
    return ret;
}

AUGSYS_API int
aug_ioeventmask(aug_mplexer_t mplexer, int fd)
{
    return external_(mplexer->pollfds_[fd].events);
}

AUGSYS_API int
aug_ioevents(aug_mplexer_t mplexer, int fd)
{
    return external_(mplexer->pollfds_[fd].revents);
}

AUGSYS_API int
aug_mplexerpipe(int fds[2])
{
    return aug_pipe(fds);
}
