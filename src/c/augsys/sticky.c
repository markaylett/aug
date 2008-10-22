/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/sticky.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/base.h" /* aug_tlx */

AUGSYS_API aug_result
aug_initsticky(struct aug_sticky* sticky, aug_muxer_t muxer, aug_md md,
               unsigned short mask)
{
    sticky->muxer_ = muxer;
    sticky->md_ = md;
    sticky->events_ = 0;
    return aug_setsticky(sticky, mask);
}

AUGSYS_API void
aug_termsticky(struct aug_sticky* sticky)
{
    aug_setmdeventmask(sticky->muxer_, sticky->md_, 0);
    sticky->muxer_ = NULL;
    sticky->md_ = AUG_BADMD;
    sticky->events_ = 0;
}

AUGSYS_API aug_result
aug_setsticky(struct aug_sticky* sticky, unsigned short mask)
{
    /* Remember original event mask before setting new. */

    unsigned short orig = aug_getmdeventmask(sticky->muxer_, sticky->md_);

    /* Always want exceptions and readability. */

    aug_verify(aug_setmdeventmask(sticky->muxer_, sticky->md_,
                                  AUG_MDEVENTRD | AUG_MDEVENTEX | mask));

    /* Only read and write are sticky. */

    orig &= AUG_MDEVENTRDWR;
    mask &= AUG_MDEVENTRDWR;

    /* Remove events that are not in the new mask. */

    sticky->events_ &= mask;

    /* Add events that were not in the original. */

    sticky->events_ |= (mask & ~orig);

    AUG_CTXDEBUG2(aug_tlx, "set sticky: mask=[%u], sticky=[%u]",
                  (unsigned)mask, (unsigned)sticky->events_);
    return AUG_SUCCESS;
}

AUGSYS_API void
aug_stickyrd(struct aug_sticky* sticky, aug_rsize rsize, size_t expected)
{
    /* Unset sticky event when not all bytes are read. */

    if (AUG_RESULT(rsize) != expected)
        sticky->events_ &= ~AUG_MDEVENTRD;
}

AUGSYS_API void
aug_stickywr(struct aug_sticky* sticky, aug_rsize rsize, size_t expected)
{
    /* Unset sticky event when not all bytes are read. */

    if (AUG_RESULT(rsize) != expected)
        sticky->events_ &= ~AUG_MDEVENTWR;
}

AUGSYS_API unsigned short
aug_getsticky(struct aug_sticky* sticky)
{
    unsigned short events = aug_getmdevents(sticky->muxer_, sticky->md_);

    /* Set sticky events. */

    sticky->events_ |= (events & AUG_MDEVENTRDWR);
    return sticky->events_ | events;
}
