/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/time.h"   /* aug_tvtoms() */
#include "augsys/unistd.h" /* aug_pipe() */

#include "augctx/base.h"
#include "augctx/defs.h"   /* AUG_MAX */
#include "augctx/errinfo.h"

#include <errno.h>

#if HAVE_POLLx
# include <sys/poll.h>
# define INIT_SIZE_ 64

/* POLLIN   - There is data to read.
   POLLPRI  - There is urgent data to read.
   POLLOUT  - Writing now will not block.
   POLLERR  - Error condition.
   POLLHUP  - Hung up.
   POLLNVAL - Invalid request: fd not open. */

#define POLLEX_ (POLLPRI | POLLERR | POLLNVAL)

struct aug_muxer_ {
    aug_mpool* mpool_;
    struct pollfd* pollfds_;
    size_t nfds_, size_;
};

static unsigned short
external_(short src)
{
    unsigned short dst = 0;

    if (src & (POLLHUP | POLLIN))
        dst |= AUG_MDEVENTRD;

    if (src & POLLOUT)
        dst |= AUG_MDEVENTWR;

    if (src & POLLEX_)
        dst |= AUG_MDEVENTEX;

    return dst;
}

static short
internal_(unsigned short src)
{
    short dst = 0;

    /* POLLERR, POLLHUP and POLLNVAL are implicit. */

    if (src & AUG_MDEVENTRD)
        dst |= (POLLHUP | POLLIN);

    if (src & AUG_MDEVENTWR)
        dst |= POLLOUT;

    if (src & AUG_MDEVENTEX)
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

static aug_result
resize_(aug_muxer_t muxer, size_t size)
{
    struct pollfd* ptr = aug_reallocmem(muxer->mpool_, muxer->pollfds_,
                                        sizeof(struct pollfd) * size);
    if (!ptr)
        return AUG_FAILERROR;

    initpollfds_(ptr + muxer->size_, size - muxer->size_);
    muxer->pollfds_ = ptr;
    muxer->size_ = size;
    return AUG_SUCCESS;
}

AUGSYS_API aug_muxer_t
aug_createmuxer(aug_mpool* mpool)
{
    aug_muxer_t muxer = aug_allocmem(mpool, sizeof(struct aug_muxer_));
    if (!muxer)
        return NULL;

    muxer->mpool_ = mpool;
    muxer->pollfds_ = NULL;
    muxer->nfds_ = muxer->size_ = 0;

    if (AUG_ISFAIL(resize_(muxer, INIT_SIZE_))) {
        aug_freemem(mpool, muxer);
        return NULL;
    }

    aug_retain(mpool);
    return muxer;
}

AUGSYS_API void
aug_destroymuxer(aug_muxer_t muxer)
{
    aug_mpool* mpool = muxer->mpool_;
    aug_freemem(mpool, muxer->pollfds_);
    aug_freemem(mpool, muxer);
    aug_release(mpool);
}

AUGSYS_API aug_result
aug_setmdeventmask(aug_muxer_t muxer, aug_md md, unsigned short mask)
{
    struct pollfd* ptr;

    if (mask & ~AUG_MDEVENTALL) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid mdevent mask [%d]"), (int)mask);
        return AUG_FAILERROR;
    }

    if (muxer->size_ <= md)
        aug_verify(resize_(muxer, AUG_MAX(md + 1, muxer->size_ * 2)));

    ptr = muxer->pollfds_ + md;
    if (mask) {

        ptr->fd = md;
        ptr->events = internal_(mask);

        if (muxer->nfds_ <= md)
            muxer->nfds_ = md + 1;

    } else {

        initpollfd_(ptr);

        if (muxer->nfds_ == md + 1) {

            for (--md; md >= 0 && -1 == muxer->pollfds_[md].fd; --md)
                ;
            muxer->nfds_ = md + 1;
        }
    }
    return AUG_SUCCESS;
}

AUGSYS_API aug_rint
aug_waitmdevents(aug_muxer_t muxer, const struct timeval* timeout)
{
    int ms, ret;

    /* A negative value means infinite timeout. */

    ms = timeout ? aug_tvtoms(timeout) : -1;

    if (-1 == (ret = poll(muxer->pollfds_, muxer->nfds_, ms)))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_MKRESULT(ret);
}

AUGSYS_API unsigned short
aug_getmdeventmask(aug_muxer_t muxer, aug_md md)
{
    return external_(muxer->pollfds_[md].events);
}

AUGSYS_API unsigned short
aug_getmdevents(aug_muxer_t muxer, aug_md md)
{
    return external_(muxer->pollfds_[md].revents);
}

#else /* !HAVE_POLL */
# include <sys/select.h>

struct set_ {
    fd_set rd_, wr_, ex_;
};

struct aug_muxer_ {
    aug_mpool* mpool_;
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
external_(struct set_* p, aug_md md)
{
    unsigned short dst = 0;

    if (FD_ISSET(md, &p->rd_))
        dst |= AUG_MDEVENTRD;

    if (FD_ISSET(md, &p->wr_))
        dst |= AUG_MDEVENTWR;

    if (FD_ISSET(md, &p->ex_))
        dst |= AUG_MDEVENTEX;

    return dst;
}

static void
setmdevents_(struct set_* p, aug_md md, unsigned short mask)
{
    unsigned short cur = external_(p, md);
    unsigned short set = ~cur & mask;
    unsigned short unset = cur & ~mask;

    if (set & AUG_MDEVENTRD)
        FD_SET(md, &p->rd_);
    else if (unset & AUG_MDEVENTRD)
        FD_CLR(md, &p->rd_);

    if (set & AUG_MDEVENTWR)
        FD_SET(md, &p->wr_);
    else if (unset & AUG_MDEVENTWR)
        FD_CLR(md, &p->wr_);

    if (set & AUG_MDEVENTEX)
        FD_SET(md, &p->ex_);
    else if (unset & AUG_MDEVENTEX)
        FD_CLR(md, &p->ex_);
}

AUGSYS_API aug_muxer_t
aug_createmuxer(aug_mpool* mpool)
{
    aug_muxer_t muxer = aug_allocmem(mpool, sizeof(struct aug_muxer_));
    if (!muxer)
        return NULL;

    muxer->mpool_ = mpool;
    zeroset_(&muxer->in_);
    zeroset_(&muxer->out_);

    /* A maxfd of -1 will result in a zero nfds value. */

    muxer->maxfd_ = -1;

    aug_retain(mpool);
    return muxer;
}

AUGSYS_API void
aug_destroymuxer(aug_muxer_t muxer)
{
    aug_mpool* mpool = muxer->mpool_;
    aug_freemem(mpool, muxer);
    aug_release(mpool);
}

AUGSYS_API aug_result
aug_setmdeventmask(aug_muxer_t muxer, aug_md md, unsigned short mask)
{
    if (FD_SETSIZE <= md)
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, EMFILE);

    if (mask & ~AUG_MDEVENTALL) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid mdevent mask [%u]"), (unsigned)mask);
        return AUG_FAILERROR;
    }

    setmdevents_(&muxer->in_, md, mask);

    /* Update maxfd. */

    if (muxer->maxfd_ <= md) {

        /* Use fd a starting point to find the highest fd with events set. */

        do {

            if (FD_ISSET(md, &muxer->in_.rd_)
                || FD_ISSET(md, &muxer->in_.wr_)
                || FD_ISSET(md, &muxer->in_.ex_))
                break;

        } while (-1 != --md);

        muxer->maxfd_ = md;
    }

    return AUG_SUCCESS;
}

AUGSYS_API aug_rint
aug_waitmdevents(aug_muxer_t muxer, const struct timeval* timeout)
{
    int ret;
    muxer->out_ = muxer->in_;

    if (-1 == (ret = select(muxer->maxfd_ + 1, &muxer->out_.rd_,
                            &muxer->out_.wr_, &muxer->out_.ex_,
                            (struct timeval*)timeout)))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_MKRESULT(ret);
}

AUGSYS_API unsigned short
aug_getmdeventmask(aug_muxer_t muxer, aug_md md)
{
    return external_(&muxer->in_, md);
}

AUGSYS_API unsigned short
aug_getmdevents(aug_muxer_t muxer, aug_md md)
{
    return external_(&muxer->out_, md);
}

#endif /* !HAVE_POLL */

AUGSYS_API aug_result
aug_muxerpipe(aug_md mds[2])
{
    return aug_fpipe(mds);
}
