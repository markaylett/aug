/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_STICKY_H
#define AUGSYS_STICKY_H

#include "augsys/muxer.h"

struct aug_sticky {
    aug_muxer_t muxer_;
    aug_md md_;
    unsigned short events_;
};

AUGSYS_API aug_result
aug_initsticky(struct aug_sticky* sticky, aug_muxer_t muxer, aug_md md,
               unsigned short mask);

AUGSYS_API void
aug_termsticky(struct aug_sticky* sticky);

AUGSYS_API aug_result
aug_setsticky(struct aug_sticky* sticky, unsigned short mask);

AUGSYS_API void
aug_stickyrd(struct aug_sticky* sticky, aug_rsize rsize, size_t expected);

AUGSYS_API void
aug_stickywr(struct aug_sticky* sticky, aug_rsize rsize, size_t expected);

AUGSYS_API unsigned short
aug_getsticky(struct aug_sticky* sticky);

#endif /* AUGSYS_STICKY_H */
