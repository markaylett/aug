/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
#ifndef AUGNET_WRITER_H
#define AUGNET_WRITER_H

/**
 * @file augnet/writer.h
 *
 * Blob writer.
 */

#include "augnet/config.h"

#include "augsys/types.h"

#include "augext/blob.h"
#include "augext/mpool.h"
#include "augext/stream.h"

#include "augtypes.h"

typedef struct aug_writer_* aug_writer_t;

AUGNET_API aug_writer_t
aug_createwriter(aug_mpool* mpool);

AUGNET_API void
aug_destroywriter(aug_writer_t writer);

AUGNET_API aug_result
aug_appendwriter(aug_writer_t writer, aug_blob* blob);

/**
 * Empty if there are no more blobs to write.
 */

AUGNET_API aug_bool
aug_writerempty(aug_writer_t writer);

/**
 * Dynamically calculate total size in bytes.
 */

AUGNET_API aug_rsize
aug_writersize(aug_writer_t writer);

AUGNET_API aug_rsize
aug_writesome(aug_writer_t writer, aug_stream* stream);

#endif /* AUGNET_WRITER_H */
