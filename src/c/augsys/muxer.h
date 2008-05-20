/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_MUXER_H
#define AUGSYS_MUXER_H

/**
 * @file augsys/muxer.h
 *
 * IO multiplexer.
 */

#include "augsys/types.h"

#if !defined(_WIN32)
# include "augsys/unistd.h"
# define aug_mclose aug_fclose
# define aug_mread  aug_fread
# define aug_mwrite aug_fwrite
#else /* _WIN32 */
# include "augsys/socket.h"
# define aug_mclose aug_sclose
# define aug_mread  aug_sread
# define aug_mwrite aug_swrite
#endif /* _WIN32 */

#define AUG_FDEVENTRD    0x1
#define AUG_FDEVENTWR    0x2
#define AUG_FDEVENTEX    0x4
#define AUG_FDEVENTRDWR (AUG_FDEVENTRD   | AUG_FDEVENTWR)
#define AUG_FDEVENTALL  (AUG_FDEVENTRDWR | AUG_FDEVENTEX)
#define AUG_FDEVENTCONN (AUG_FDEVENTWR   | AUG_FDEVENTEX)

struct timeval;

typedef struct aug_muxer_* aug_muxer_t;

AUGSYS_API aug_muxer_t
aug_createmuxer(void);

AUGSYS_API int
aug_destroymuxer(aug_muxer_t muxer);

AUGSYS_API void
aug_setnowait(aug_muxer_t muxer, unsigned nowait);

AUGSYS_API int
aug_setfdeventmask(aug_muxer_t muxer, aug_md md, unsigned short mask);

/**
 * @return Either positive if events have been signalled, zero on timeout, or
 * a negative value on error.  A positive return may not be representative of
 * the actual number of signalled events.
 *
 * If #SA_RESTART has been set for an interrupting signal, it is
 * implementation dependant whether select()/poll() restart or return with
 * #EINTR set.
 */

AUGSYS_API int
aug_waitfdevents(aug_muxer_t muxer, const struct timeval* timeout);

AUGSYS_API int
aug_fdeventmask(aug_muxer_t muxer, aug_md md);

AUGSYS_API int
aug_fdevents(aug_muxer_t muxer, aug_md md);

/**
 * Creates a pipe or socket-pair suitable for use with muxer.  On Windows,
 * only socket descriptors can be used with select(), therefore,
 * aug_muxerpipe() will return a socket-pair.
 *
 * @see aug_socketpair().
 */

AUGSYS_API int
aug_muxerpipe(aug_md mds[2]);

#endif /* AUGSYS_MUXER_H */
