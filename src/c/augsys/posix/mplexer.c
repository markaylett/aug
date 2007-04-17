/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/defs.h"   /* AUG_MAX */
#include "augsys/errinfo.h"
#include "augsys/time.h"   /* aug_tvtoms() */
#include "augsys/unistd.h" /* aug_pipe() */

#include <errno.h>
#include <stdlib.h>        /* malloc() */

#if HAVE_POLL
# include <sys/poll.h>
# define INIT_SIZE_ 64

struct aug_mplexer_ {
    struct pollfd* pollfds_;
    size_t nfds_, size_;
};

static unsigned short
external_(short src)
{
    unsigned short dst = 0;

    if (src & (POLLHUP | POLLIN))
        dst |= AUG_FDEVENTRD;

    if (src & POLLOUT)
        dst |= AUG_FDEVENTWR;

    if (src & POLLPRI)
        dst |= AUG_FDEVENTEX;

    return dst;
}

static short
internal_(unsigned short src)
{
    short dst = 0;

    if (src & AUG_FDEVENTRD)
        dst |= (POLLHUP | POLLIN);

    if (src & AUG_FDEVENTWR)
        dst |= POLLOUT;

    if (src & AUG_FDEVENTEX)
        dst |= POLLPRI;

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
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
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
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
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
aug_destroymplexer(aug_mplexer_t mplexer)
{
    free(mplexer->pollfds_);
    free(mplexer);
    return 0;
}

AUGSYS_API int
aug_setfdeventmask(aug_mplexer_t mplexer, int fd, unsigned short mask)
{
    struct pollfd* ptr;

    if (mask & ~AUG_FDEVENTALL) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid fdevent mask '%d'"), (int)mask);
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
aug_waitfdevents(aug_mplexer_t mplexer, const struct timeval* timeout)
{
    int ms, ret;

    ms = timeout ? aug_tvtoms(timeout) : -1;

    if (-1 == (ret = poll(mplexer->pollfds_, mplexer->nfds_, ms))) {

        if (EINTR == aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno))
            ret = AUG_RETINTR;
    }
    return ret;
}

AUGSYS_API int
aug_fdeventmask(aug_mplexer_t mplexer, int fd)
{
    return external_(mplexer->pollfds_[fd].events);
}

AUGSYS_API int
aug_fdevents(aug_mplexer_t mplexer, int fd)
{
    return external_(mplexer->pollfds_[fd].revents);
}

#else /* !HAVE_POLL */
# include <sys/select.h>

struct set_ {
    fd_set rd_, wr_, ex_;
};

struct aug_mplexer_ {
    struct set_ in_, out_;
    int maxfd_;
};

static void
zeroset_(struct set_* p)
{
    FD_ZERO(&p->rd_);
    FD_ZERO(&p->wr_);
    FD_ZERO(&p->ex_);
}

static unsigned short
external_(struct set_* p, int fd)
{
    unsigned short dst = 0;

    if (FD_ISSET(fd, &p->rd_))
        dst |= AUG_FDEVENTRD;

    if (FD_ISSET(fd, &p->wr_))
        dst |= AUG_FDEVENTWR;

    if (FD_ISSET(fd, &p->ex_))
        dst |= AUG_FDEVENTEX;

    return dst;
}

static void
setfdevents_(struct set_* p, int fd, unsigned short mask)
{
    unsigned short cur = external_(p, fd);
    unsigned short set = ~cur & mask;
    unsigned short unset = cur & ~mask;

    if (set & AUG_FDEVENTRD)
        FD_SET(fd, &p->rd_);
    else if (unset & AUG_FDEVENTRD)
        FD_CLR(fd, &p->rd_);

    if (set & AUG_FDEVENTWR)
        FD_SET(fd, &p->wr_);
    else if (unset & AUG_FDEVENTWR)
        FD_CLR(fd, &p->wr_);

    if (set & AUG_FDEVENTEX)
        FD_SET(fd, &p->ex_);
    else if (unset & AUG_FDEVENTEX)
        FD_CLR(fd, &p->ex_);
}

AUGSYS_API aug_mplexer_t
aug_createmplexer(void)
{
    aug_mplexer_t mplexer = malloc(sizeof(struct aug_mplexer_));
    if (!mplexer) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    zeroset_(&mplexer->in_);
    zeroset_(&mplexer->out_);

    /* A maxfd of -1 will result in a zero nfds value. */

    mplexer->maxfd_ = -1;
    return mplexer;
}

AUGSYS_API int
aug_destroymplexer(aug_mplexer_t mplexer)
{
    free(mplexer);
    return 0;
}

AUGSYS_API int
aug_setfdeventmask(aug_mplexer_t mplexer, int fd, unsigned short mask)
{
    if (FD_SETSIZE <= fd) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, EMFILE);
        return -1;
    }

    if (mask & ~AUG_FDEVENTALL) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid fdevent mask"));
        return -1;
    }

    setfdevents_(&mplexer->in_, fd, mask);

    /* Update maxfd. */

    if (mplexer->maxfd_ <= fd) {

        /* Use fd a starting point to find the highest fd with events set. */

        do {

            if (FD_ISSET(fd, &mplexer->in_.rd_)
                || FD_ISSET(fd, &mplexer->in_.wr_)
                || FD_ISSET(fd, &mplexer->in_.ex_))
                break;

        } while (-1 != --fd);

        mplexer->maxfd_ = fd;
    }

    return 0;
}

AUGSYS_API int
aug_waitfdevents(aug_mplexer_t mplexer, const struct timeval* timeout)
{
    int ret;
    mplexer->out_ = mplexer->in_;

    if (-1 == (ret = select(mplexer->maxfd_ + 1, &mplexer->out_.rd_,
                            &mplexer->out_.wr_, &mplexer->out_.ex_,
                            (struct timeval*)timeout))) {

        if (EINTR == aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno))
            ret = AUG_RETINTR;
    }
    return ret;
}

AUGSYS_API int
aug_fdeventmask(aug_mplexer_t mplexer, int fd)
{
    return external_(&mplexer->in_, fd);
}

AUGSYS_API int
aug_fdevents(aug_mplexer_t mplexer, int fd)
{
    return external_(&mplexer->out_, fd);
}

#endif /* !HAVE_POLL */

AUGSYS_API int
aug_mplexerpipe(int fds[2])
{
    return aug_pipe(fds);
}
