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

/**
   Muxer events.

   Notes from Linux man pages:

   send(): The select(2) or poll(2) call may be used to determine when it is
   possible to send more data.

   recv(): The select(2) or poll(2) call may be used to determine when more
   data arrives.

   accept(): In order to be notified of incoming connections on a socket, you
   can use select(2) or poll(2).  A readable event will be delivered when a
   new connection is attempted and you may then call accept() to get a socket
   for that connection.

   connect(): It is possible to select(2) or poll(2) for completion by
   selecting the socket for writing.  After select(2) indicates writability,
   use getsockopt(2) to read the SO_ERROR option at level SOL_SOCKET to
   determine whether connect() completed successfully (SO_ERROR is zero) or
   unsuccessfully.

   Winsock specific: failure of connection attempt is communicated as an
   exception event.

   FIXME: is this safe for non-winsock clients?

   All muxer implementations are assumed to be edge-triggered.
*/

#define AUG_MDEVENTRD    0x1
#define AUG_MDEVENTWR    0x2
#define AUG_MDEVENTEX    0x4
#define AUG_MDEVENTRDWR (AUG_MDEVENTRD   | AUG_MDEVENTWR)
#define AUG_MDEVENTRDEX (AUG_MDEVENTRD   | AUG_MDEVENTEX)
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

/**
 * Wait for events.
 *
 * @param timeout Optional timeout value; a null timeout causes the function
 * to block until an event occurs.
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

/**
 * Poll events without waiting.
 */

AUGSYS_API aug_rint
aug_pollmdevents(aug_muxer_t muxer);

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

AUGSYS_API const char*
aug_eventlabel(unsigned short events);

#endif /* AUGSYS_MUXER_H */
