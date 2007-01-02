/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_MPLEXER_H
#define AUGSYS_MPLEXER_H

#include "augsys/config.h"

#define AUG_IOEVENTRD    0x1
#define AUG_IOEVENTWR    0x2
#define AUG_IOEVENTRDWR (AUG_IOEVENTRD | AUG_IOEVENTWR)

struct timeval;

typedef struct aug_mplexer_* aug_mplexer_t;

AUGSYS_API aug_mplexer_t
aug_createmplexer(void);

AUGSYS_API int
aug_freemplexer(aug_mplexer_t mplexer);

AUGSYS_API int
aug_setioeventmask(aug_mplexer_t mplexer, int fd, unsigned short mask);

/**
   Returns either the total number of descriptors set, zero on timeout, or a
   negative value on error.

   If #SA_RESTART has been set for an interrupting signal, it is
   implementation dependant whether select()/poll() restart or return with
   #EINTR set.
*/

AUGSYS_API int
aug_waitioevents(aug_mplexer_t mplexer, const struct timeval* timeout);

AUGSYS_API int
aug_ioeventmask(aug_mplexer_t mplexer, int fd);

AUGSYS_API int
aug_ioevents(aug_mplexer_t mplexer, int fd);

AUGSYS_API int
aug_mplexerpipe(int fds[2]);

#endif /* AUGSYS_MPLEXER_H */
