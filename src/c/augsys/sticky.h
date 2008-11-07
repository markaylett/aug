/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_STICKY_H
#define AUGSYS_STICKY_H

#include "augsys/muxer.h"

/**
   Sticky events to simplify support for edge-triggered interfaces.

   Edge- versus level-triggered:


   1     +----+    +----+    +----+
         |    |    |    |    |    |
         |    |    |    |    |    |
   0 ----+    +----+    +----+    +----

   horizontal: level
   vertical:   edge

   Only read and write are sticky.  Read and writes events should remain so
   until explicitly cleared.  Moreover, changing the mask should not clear any
   sticky events; if a mask is reverted, the original sticky events will
   become visible again.

   Invariants:

   Muxer-mask is zero when sticky events are set.

   Sticky events are zero when muxer-mask is set.

   Muxer-mask and sticky events must be a subset of mask.
 */

struct aug_sticky {
    aug_muxer_t muxer_;
    aug_md md_;
    unsigned short mask_, internal_;
};

AUGSYS_API aug_result
aug_initsticky(struct aug_sticky* sticky, aug_muxer_t muxer, aug_md md,
               unsigned short mask);

AUGSYS_API void
aug_termsticky(struct aug_sticky* sticky);

AUGSYS_API aug_result
aug_clearsticky(struct aug_sticky* sticky, unsigned short mask);

AUGSYS_API aug_result
aug_setsticky(struct aug_sticky* sticky, unsigned short mask);

AUGSYS_API unsigned short
aug_getsticky(struct aug_sticky* sticky);

#endif /* AUGSYS_STICKY_H */
