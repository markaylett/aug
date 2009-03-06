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
#ifndef AUGSYS_STREAM_H
#define AUGSYS_STREAM_H

#include "augsys/config.h"
#include "augsys/types.h"

#include "augext/stream.h"
#include "augext/mpool.h"

/**
 * Create file stream.
 *
 * Does not take ownership of file handle.
 */

AUGSYS_API aug_stream*
aug_createfstream(aug_mpool* mpool, aug_fd fd);

/**
 * Create socket stream.
 *
 * Does not take ownership of socket handle.
 */

AUGSYS_API aug_stream*
aug_createsstream(aug_mpool* mpool, aug_sd sd);

#endif /* AUGSYS_STREAM_H */
