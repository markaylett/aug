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

#include "augext/mpool.h"

#define AUG_MDEVENTRD    0x1
#define AUG_MDEVENTWR    0x2
#define AUG_MDEVENTEX    0x4
#define AUG_MDEVENTRDWR (AUG_MDEVENTRD   | AUG_MDEVENTWR)
#define AUG_MDEVENTALL  (AUG_MDEVENTRDWR | AUG_MDEVENTEX)
#define AUG_MDEVENTCONN (AUG_MDEVENTWR   | AUG_MDEVENTEX)

struct timeval;

typedef struct aug_muxer_* aug_muxer_t;

AUGSYS_API aug_muxer_t
aug_createmuxer(aug_mpool* mpool);

AUGSYS_API void
aug_destroymuxer(aug_muxer_t muxer);

AUGSYS_API aug_result
aug_setmdeventmask(aug_muxer_t muxer, aug_md md, unsigned short mask);

AUGSYS_API void
aug_setmdevents(aug_muxer_t muxer, int delta);

/**
 * Wait for events.
 *
 * @param timeout Optional timeout value.
 *
 * @return Either positive if events have been signalled, zero on timeout, or
 * a negative value on error.  A positive return may not be representative of
 * the actual number of signalled events.
 *
 * If #SA_RESTART has been set for an interrupting signal, it is
 * implementation dependant whether select()/poll() restart or return with
 * #EINTR set.
 */

AUGSYS_API aug_rint
aug_waitmdevents(aug_muxer_t muxer, const struct timeval* timeout);

AUGSYS_API unsigned short
aug_getmdeventmask(aug_muxer_t muxer, aug_md md);

/**
 * Return events set for @a md.
 *
 * @param muxer The multiplexer.
 * @param md A multiplexer-managed descriptor.
 *
 * @return Events set.
 */

AUGSYS_API unsigned short
aug_getmdevents(aug_muxer_t muxer, aug_md md);

/**
 * Creates a pipe or socket-pair suitable for use with muxer.  On Windows,
 * only socket descriptors can be used with select(), therefore,
 * aug_muxerpipe() will return a socket-pair.
 *
 * @see aug_socketpair().
 */

AUGSYS_API aug_result
aug_muxerpipe(aug_md mds[2]);

#endif /* AUGSYS_MUXER_H */
