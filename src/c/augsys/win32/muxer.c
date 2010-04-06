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
    if (FD_SETSIZE == muxer->out_.rd_.fd_count) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAEMFILE);
        return -1;
    }

    if (mask & ~AUG_MDEVENTALL) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid mdevent mask"));
        return -1;
    }

    muxer->bits_ += setmdevents_(&muxer->in_, md, mask);
    return 0;
}

AUGSYS_API aug_rint
aug_waitmdevents_I(aug_muxer_t muxer, const struct aug_timeval* timeout)
{
    int ret;

    muxer->out_ = muxer->in_;

    if (0 == muxer->bits_) {
        Sleep(timeout ? aug_tvtoms(timeout) : INFINITE);
        return 0;
    }

    if (timeout) {

        struct timeval tv;
        tv.tv_sec = timeout->tv_sec;
        tv.tv_usec = timeout->tv_usec;

        ret = select(-1, &muxer->out_.rd_, &muxer->out_.wr_,
                     &muxer->out_.ex_, &tv);

    } else {

        ret = select(-1, &muxer->out_.rd_, &muxer->out_.wr_,
                     &muxer->out_.ex_, NULL);
    }

    /* Note: WinSock ignores the nfds argument. */

    if (SOCKET_ERROR == ret) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    return ret;
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

AUGSYS_API aug_result
aug_muxerpipe_BIN(aug_md mds[2])
{
    aug_md sds[2];

    if (aug_socketpair_BIN(AF_UNIX, SOCK_STREAM, 0, sds) < 0)
        return -1;

    if (aug_ssetnonblock_BI(sds[0], AUG_TRUE) < 0
        || aug_ssetnonblock_BI(sds[1], AUG_TRUE) < 0) {
        aug_sclose(sds[0]);
        aug_sclose(sds[1]);
        return -1;
    }

    mds[0] = sds[0];
    mds[1] = sds[1];
    return 0;
}
