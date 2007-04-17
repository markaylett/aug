/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_MPLEXER_H
#define AUGSYS_MPLEXER_H

#include "augsys/config.h"

#define AUG_FDEVENTRD    0x1
#define AUG_FDEVENTWR    0x2
#define AUG_FDEVENTEX    0x4
#define AUG_FDEVENTRDWR (AUG_FDEVENTRD   | AUG_FDEVENTWR)
#define AUG_FDEVENTALL  (AUG_FDEVENTRDWR | AUG_FDEVENTEX)

struct timeval;

typedef struct aug_mplexer_* aug_mplexer_t;

AUGSYS_API aug_mplexer_t
aug_createmplexer(void);

AUGSYS_API int
aug_destroymplexer(aug_mplexer_t mplexer);

AUGSYS_API int
aug_setfdeventmask(aug_mplexer_t mplexer, int fd, unsigned short mask);

/**
   \return either the total number of descriptors set, zero on timeout, or a
   negative value on error.

   If #SA_RESTART has been set for an interrupting signal, it is
   implementation dependant whether select()/poll() restart or return with
   #EINTR set.
*/

AUGSYS_API int
aug_waitfdevents(aug_mplexer_t mplexer, const struct timeval* timeout);

AUGSYS_API int
aug_fdeventmask(aug_mplexer_t mplexer, int fd);

AUGSYS_API int
aug_fdevents(aug_mplexer_t mplexer, int fd);

/**
   Creates a pipe or socket-pair suitable for use with mplexer.  On Windows,
   only socket descriptors can be used with select(), therefore,
   aug_mplexerpipe() will return a socket-pair.

   \sa aug_socketpair().

 */

AUGSYS_API int
aug_mplexerpipe(int fds[2]);

#endif /* AUGSYS_MPLEXER_H */
