/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
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
