/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/socket.h"
#include "augsys/time.h" /* aug_tvtoms() */
#include "augsys/windows.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"

#include <io.h>

struct set_ {
    fd_set rd_, wr_, ex_;
};

struct aug_muxer_ {
    aug_mpool* mpool_;
    struct set_ in_, out_;
    size_t bits_;
    int ready_;
};

static void
zeroset_(struct set_* p)
{
    FD_ZERO(&p->rd_);
    FD_ZERO(&p->wr_);
    FD_ZERO(&p->ex_);
}

static unsigned short
external_(struct set_* p, SOCKET h)
{
    unsigned short dst = 0;

    if (FD_ISSET(h, &p->rd_))
        dst |= AUG_MDEVENTRD;

    if (FD_ISSET(h, &p->wr_))
        dst |= AUG_MDEVENTWR;

    if (FD_ISSET(h, &p->ex_))
        dst |= AUG_MDEVENTEX;

    return dst;
}

static int
setmdevents_(struct set_* p, SOCKET h, unsigned short mask)
{
    unsigned short cur = external_(p, h);
    unsigned short set = ~cur & mask;
    unsigned short unset = cur & ~mask;
    int bits = 0;

    if (set & AUG_MDEVENTRD) {

        FD_SET(h, &p->rd_);
        ++bits;

    } else if (unset & AUG_MDEVENTRD) {

        FD_CLR(h, &p->rd_);
        --bits;
    }

    if (set & AUG_MDEVENTWR) {

        FD_SET(h, &p->wr_);
        ++bits;

    } else if (unset & AUG_MDEVENTWR) {

        FD_CLR(h, &p->wr_);
        --bits;
    }

    if (set & AUG_MDEVENTEX) {

        FD_SET(h, &p->ex_);
        ++bits;

    } else if (unset & AUG_MDEVENTEX) {

        FD_CLR(h, &p->ex_);
        --bits;
    }

    return bits;
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
    muxer->bits_ = 0;
    muxer->ready_ = 0;

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

AUGSYS_API int
aug_setmdeventmask(aug_muxer_t muxer, aug_md md, unsigned short mask)
{
    if (FD_SETSIZE == muxer->out_.rd_.fd_count) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, WSAEMFILE);
        return -1;
    }

    if (mask & ~AUG_MDEVENTALL) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid mdevent mask"));
        return -1;
    }

    muxer->bits_ += setmdevents_(&muxer->in_, md, mask);
    return 0;
}

AUGSYS_API void
aug_setmdevents(aug_muxer_t muxer, int delta)
{
    muxer->ready_ += delta;
}

AUGSYS_API int
aug_waitmdevents(aug_muxer_t muxer, const struct timeval* timeout)
{
    int ret, ready = muxer->ready_;
    muxer->ready_ = 0;

    if (0 < ready) {
        /* Recurse. */
        ret = aug_waitmdevents(muxer, &NOWAIT_);
        if (0 <= ret)
            ret += ready; /* At least one. */
        return ret;
    }

    muxer->out_ = muxer->in_;

    if (0 == muxer->bits_) {
        Sleep(timeout ? aug_tvtoms(timeout) : INFINITE);
        return 0;
    }

    /* Note: WinSock ignores the nfds argument. */

    if (SOCKET_ERROR ==
        (ret = select(-1, &muxer->out_.rd_, &muxer->out_.wr_,
                      &muxer->out_.ex_, timeout))) {

        ret = aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                  WSAGetLastError());
    }
    return ret;
}

AUGSYS_API int
aug_getmdeventmask(aug_muxer_t muxer, aug_md md)
{
    return external_(&muxer->in_, md);
}

AUGSYS_API int
aug_getmdevents(aug_muxer_t muxer, aug_md md)
{
    return external_(&muxer->out_, md);
}

AUGSYS_API int
aug_muxerpipe(aug_md mds[2])
{
    return aug_socketpair(AF_UNIX, SOCK_STREAM, 0, mds);
}
