/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errno.h"
#include "augsys/socket.h"
#include "augsys/time.h" /* aug_tvtoms() */
#include "augsys/windows.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <io.h>
#include <stdlib.h>      /* malloc() */

struct set_ {
    fd_set rd_, wr_, ex_;
};

struct aug_muxer_ {
    struct set_ in_, out_;
    size_t bits_;
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
        dst |= AUG_FDEVENTRD;

    if (FD_ISSET(h, &p->wr_))
        dst |= AUG_FDEVENTWR;

    if (FD_ISSET(h, &p->ex_))
        dst |= AUG_FDEVENTEX;

    return dst;
}

static int
setfdevents_(struct set_* p, SOCKET h, unsigned short mask)
{
    unsigned short cur = external_(p, h);
    unsigned short set = ~cur & mask;
    unsigned short unset = cur & ~mask;
    int bits = 0;

    if (set & AUG_FDEVENTRD) {

        FD_SET(h, &p->rd_);
        ++bits;

    } else if (unset & AUG_FDEVENTRD) {

        FD_CLR(h, &p->rd_);
        --bits;
    }

    if (set & AUG_FDEVENTWR) {

        FD_SET(h, &p->wr_);
        ++bits;

    } else if (unset & AUG_FDEVENTWR) {

        FD_CLR(h, &p->wr_);
        --bits;
    }

    if (set & AUG_FDEVENTEX) {

        FD_SET(h, &p->ex_);
        ++bits;

    } else if (unset & AUG_FDEVENTEX) {

        FD_CLR(h, &p->ex_);
        --bits;
    }

    return bits;
}

AUGSYS_API aug_muxer_t
aug_createmuxer(void)
{
    aug_muxer_t muxer = malloc(sizeof(struct aug_muxer_));
    if (!muxer) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    zeroset_(&muxer->in_);
    zeroset_(&muxer->out_);
    muxer->bits_ = 0;
    return muxer;
}

AUGSYS_API int
aug_destroymuxer(aug_muxer_t muxer)
{
    free(muxer);
    return 0;
}

AUGSYS_API int
aug_setfdeventmask(aug_muxer_t muxer, int fd, unsigned short mask)
{
    if (FD_SETSIZE == muxer->out_.rd_.fd_count) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, WSAEMFILE);
        return -1;
    }

    if (mask & ~AUG_FDEVENTALL) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid fdevent mask"));
        return -1;
    }

    muxer->bits_ += setfdevents_(&muxer->in_, _get_osfhandle(fd), mask);
    return 0;
}

AUGSYS_API int
aug_waitfdevents(aug_muxer_t muxer, const struct timeval* timeout)
{
    int ret;
    muxer->out_ = muxer->in_;

    if (0 == muxer->bits_) {
        Sleep(timeout ? aug_tvtoms(timeout) : INFINITE);
        return 0;
    }

    /* Note: WinSock ignores the nfds argument. */

    if (SOCKET_ERROR ==
        (ret = select(-1, &muxer->out_.rd_, &muxer->out_.wr_,
                      &muxer->out_.ex_, timeout))) {
        if (WSAEINTR == aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                            WSAGetLastError()))
            ret = AUG_RETINTR;
    }
    return ret;
}

AUGSYS_API int
aug_fdeventmask(aug_muxer_t muxer, int fd)
{
    return external_(&muxer->in_, _get_osfhandle(fd));
}

AUGSYS_API int
aug_fdevents(aug_muxer_t muxer, int fd)
{
    return external_(&muxer->out_, _get_osfhandle(fd));
}

AUGSYS_API int
aug_muxerpipe(int fds[2])
{
    int local[2];

    if (-1 == aug_socketpair(AF_UNIX, SOCK_STREAM, 0, local))
        return -1;

    fds[0] = local[0];
    fds[1] = local[1];
    return 0;
}
