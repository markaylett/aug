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
#define AUGSYS_BUILD
#include "augsys/sticky.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/base.h" /* aug_tlx */

/* External events are the events visible to the outside world.  Externally
   visible events are always a subset of the current mask.  In contrast,
   internal events are the set of all sticky events that have not been
   cleared. */

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

        if (aug_setmdeventmask(sticky->muxer_, sticky->md_,
                               sticky->mask_) < 0)
            return -1;
    }

    return 0;
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

    /* Set muxer mask if there are no external events. */

    if (!EXTERNAL_(sticky)) {

        AUG_CTXDEBUG3(aug_tlx,
                      "no sticky events, setting muxer-mask: mask=[%s]",
                      aug_eventlabel(mask));

    } else {

        AUG_CTXDEBUG3(aug_tlx, "sticky events, clearing muxer-mask");

        /* Invariant: muxer-mask is zero when sticky events are set. */

        mask = 0;
    }

    if (aug_setmdeventmask(sticky->muxer_, sticky->md_, mask) < 0)
        return -1;

    return 0;
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
