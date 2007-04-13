/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_MPLEXER_H
#define AUGSYS_MPLEXER_H

#include "augsys/config.h"

#define AUG_IOEVENTRD    0x1
#define AUG_IOEVENTWR    0x2
#define AUG_IOEVENTEX    0x4
#define AUG_IOEVENTRDWR (AUG_IOEVENTRD | AUG_IOEVENTWR)
#define AUG_IOEVENTALL  (AUG_IOEVENTRDWR | AUG_IOEVENTEX)

struct timeval;

typedef struct aug_mplexer_* aug_mplexer_t;

AUGSYS_API aug_mplexer_t
aug_createmplexer(void);

AUGSYS_API int
aug_destroymplexer(aug_mplexer_t mplexer);

AUGSYS_API int
aug_setioeventmask(aug_mplexer_t mplexer, int fd, unsigned short mask);

/**
   \return either the total number of descriptors set, zero on timeout, or a
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

/**
   Creates a pipe or socket-pair suitable for use with mplexer.  On Windows,
   only socket descriptors can be used with select(), therefore,
   aug_mplexerpipe() will return a socket-pair.

   \sa aug_socketpair().

 */

AUGSYS_API int
aug_mplexerpipe(int fds[2]);

#endif /* AUGSYS_MPLEXER_H */
