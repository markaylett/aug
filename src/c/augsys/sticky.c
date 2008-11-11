/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/sticky.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/base.h" /* aug_tlx */

/* External events are sticky events that are visible to the outside world.
   Externally visible events are always a subset of the current mask.  In
   contrast, internal events are the set of all sticky events that have not
   been cleared. */

#define EXTERNAL_(x) ((x)->internal_ & (x)->mask_)

AUGSYS_API aug_result
aug_initsticky(struct aug_sticky* sticky, aug_muxer_t muxer, aug_md md,
               unsigned short mask)
{
    sticky->muxer_ = muxer;
    sticky->md_ = md;
    sticky->mask_ = 0;
    sticky->internal_ = 0;
    return aug_setsticky(sticky, mask);
}

AUGSYS_API void
aug_termsticky(struct aug_sticky* sticky)
{
    /* Clear muxer-mask on destruction. */

    aug_setmdeventmask(sticky->muxer_, sticky->md_, 0);

    sticky->muxer_ = NULL;
    sticky->md_ = AUG_BADMD;
    sticky->mask_ = 0;
    sticky->internal_ = 0;
}

AUGSYS_API aug_result
aug_clearsticky(struct aug_sticky* sticky, unsigned short mask)
{
    /* Clear sticky events according to mask argument. */

    sticky->internal_ &= ~mask;

    /* Set muxer mask if no external events. */

    if (sticky->mask_ && !EXTERNAL_(sticky)) {

        /* Invariant: sticky events are zero when muxer-mask is set. */

        AUG_CTXDEBUG3(aug_tlx,
                      "no sticky events, setting muxer-mask: mask=[%s]",
                      aug_eventlabel(sticky->mask_));

        aug_verify(aug_setmdeventmask(sticky->muxer_, sticky->md_,
                                      sticky->mask_));
    }

    return AUG_SUCCESS;
}

AUGSYS_API aug_result
aug_setsticky(struct aug_sticky* sticky, unsigned short mask)
{
    /* Sticky events in new mask. */

    unsigned short events = mask & AUG_MDEVENTRDWR;

    /* Set sticky events that were not in the original mask.  This is done so
       that explicitly cleared events are not set. */

    sticky->internal_ |= (events & ~sticky->mask_);

    /* Done using original mask; set new mask. */

    sticky->mask_ = mask;

    /* Clear muxer mask if external events. */

    if (!EXTERNAL_(sticky)) {

        AUG_CTXDEBUG3(aug_tlx,
                      "no sticky events, setting muxer-mask: mask=[%s]",
                      aug_eventlabel(mask));

    } else {

        AUG_CTXDEBUG3(aug_tlx, "sticky events, clearing muxer-mask");

        /* Invariant: muxer-mask is zero when sticky events are set. */

        mask = 0;
    }

    aug_verify(aug_setmdeventmask(sticky->muxer_, sticky->md_, mask));

    return AUG_SUCCESS;
}

AUGSYS_API unsigned short
aug_getsticky(struct aug_sticky* sticky)
{
    unsigned short events;

    /* If the mask is zero, so too are the events. */

    if (0 == sticky->mask_)
        return 0;

    events = EXTERNAL_(sticky);

    /* Get muxer events if no external events. */

    if (sticky->mask_ && !events) {

        /* Get events from muxer. */

        events = aug_getmdevents(sticky->muxer_, sticky->md_);

        /* Save any sticky events. */

        sticky->internal_ |= (events & AUG_MDEVENTRDWR);

        /* If sticky events are now set, then clear muxer-mask.  The
           muxer-mask will not be set again until all external sticky bits
           have been cleared. */

        if (EXTERNAL_(sticky)) {

            AUG_CTXDEBUG3(aug_tlx, "sticky events, clearing muxer-mask:"
                          " muxer-events=[%s]", aug_eventlabel(events));

            aug_setmdeventmask(sticky->muxer_, sticky->md_, 0);
        }
    }

    return events;
}
