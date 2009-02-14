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
#ifndef AUGNET_TCPCONNECT_H
#define AUGNET_TCPCONNECT_H

/**
 * @file augnet/tcpconnect.h
 *
 * Non-blocking TCP establishment.
 */

#include "augnet/config.h"

#include "augsys/socket.h"

#include "augext/mpool.h"

typedef struct aug_tcpconnect_* aug_tcpconnect_t;

AUGNET_API aug_tcpconnect_t
aug_createtcpconnect(aug_mpool* mpool, const char* host, const char* serv);

AUGNET_API void
aug_destroytcpconnect(aug_tcpconnect_t conn);

/**
 * Attempt to establish connection without blocking.
 *
 * If a connection was established, then @a est flag will be set and a
 * non-blocking socket returned.
 *
 * Otherwise, the returned descriptor should be polled for #AUG_MDEVENTCONN
 * events before retrying aug_tryconnect().
 *
 * The descriptor returned between calls to aug_tryconnect() may change; this
 * occurs as the implementation cycles through the addresses associated with
 * the host.  All but the established descriptor are owned, and closed, by the
 * aug_tcpconnect_t instance.
 */

/* FIXME: change to aug_bool. */

AUGNET_API aug_sd
aug_tryconnect(aug_tcpconnect_t conn, struct aug_endpoint* ep, int* est);

#endif /* AUGNET_TCPCONNECT_H */
