/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/base.h"
#include "augsys/errno.h"
#include "augsys/socket.h"
#include "augsys/unistd.h"

#include <winsock2.h> /* select() */

#include <stdlib.h>   /* malloc() */

struct set_ {
    fd_set rd_, wr_;
};

struct aug_mplexer_ {
    struct set_ in_, out_;
    size_t bits_;
};

static void
zeroset_(struct set_* p)
{
    FD_ZERO(&p->rd_);
    FD_ZERO(&p->wr_);
}

static unsigned short
external_(struct set_* p, SOCKET h)
{
    unsigned short dst = 0;

    if (FD_ISSET(h, &p->rd_))
        dst |= AUG_EVENTRD;

    if (FD_ISSET(h, &p->wr_))
        dst |= AUG_EVENTWR;

    return dst;
}

static int
setevents_(struct set_* p, SOCKET h, unsigned short mask)
{
    unsigned short cur = external_(p, h);
    unsigned short set = ~cur & mask;
    unsigned short unset = cur & ~mask;
    int bits = 0;

    if (set & AUG_EVENTRD) {

        FD_SET(h, &p->rd_);
        ++bits;

    } else if (unset & AUG_EVENTRD) {

        FD_CLR(h, &p->rd_);
        --bits;
    }

    if (set & AUG_EVENTWR) {

        FD_SET(h, &p->wr_);
        ++bits;

    } else if (unset & AUG_EVENTWR) {

        FD_CLR(h, &p->wr_);
        --bits;
    }

    return bits;
}

AUGSYS_API aug_mplexer_t
aug_createmplexer(void)
{
    aug_mplexer_t mplexer = malloc(sizeof(struct aug_mplexer_));
    if (!mplexer)
        return NULL;

    zeroset_(&mplexer->in_);
    zeroset_(&mplexer->out_);
    mplexer->bits_ = 0;
    return mplexer;
}

AUGSYS_API int
aug_freemplexer(aug_mplexer_t mplexer)
{
    free(mplexer);
    return 0;
}

AUGSYS_API int
aug_seteventmask(aug_mplexer_t mplexer, int fd, unsigned short mask)
{
    if (mask & ~(AUG_EVENTRD | AUG_EVENTWR)) {
        errno = EINVAL;
        return -1;
    }

    mplexer->bits_ += setevents_(&mplexer->in_, _get_osfhandle(fd), mask);
    return 0;
}

AUGSYS_API int
aug_waitevents(aug_mplexer_t mplexer, const struct timeval* timeout)
{
    int ret;
    mplexer->out_ = mplexer->in_;

    if (0 == mplexer->bits_) {

        if (timeout) {
            int ms = timeout->tv_sec * 1000;
            ms += (timeout->tv_usec + 500) / 1000;
            Sleep(ms);
        } else
            Sleep(INFINITE);

        return 0;
    }

    /* Note: WinSock ignores the nfds argument. */

    if (SOCKET_ERROR ==
        (ret = select(-1, &mplexer->out_.rd_, &mplexer->out_.wr_,
                      NULL, timeout))) {

        aug_maperror(WSAGetLastError());
        if (EINTR == errno)
            ret = AUG_EINTR;
    }
    return ret;
}

AUGSYS_API int
aug_eventmask(aug_mplexer_t mplexer, int fd)
{
    return external_(&mplexer->in_, _get_osfhandle(fd));
}

AUGSYS_API int
aug_events(aug_mplexer_t mplexer, int fd)
{
    return external_(&mplexer->out_, _get_osfhandle(fd));
}

AUGSYS_API int
aug_mplexerpipe(int fds[2])
{
    int local[2];

    if (-1 == aug_socketpair(AF_UNIX, SOCK_STREAM, 0, local))
        return -1;

    fds[0] = local[0];
    fds[1] = local[1];
    return 0;
}
