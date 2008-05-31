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
#include "augext/stream.h"

typedef struct aug_writer_* aug_writer_t;

AUGNET_API aug_writer_t
aug_createwriter(void);

AUGNET_API int
aug_destroywriter(aug_writer_t writer);

AUGNET_API int
aug_appendwriter(aug_writer_t writer, aug_blob* blob);

/**
 * Empty if there are no more blobs to write.
 */

AUGNET_API int
aug_writerempty(aug_writer_t writer);

/**
 * Dynamically calculate total size in bytes.
 */

AUGNET_API ssize_t
aug_writersize(aug_writer_t writer);

AUGNET_API ssize_t
aug_writesome(aug_writer_t writer, aug_stream* stream);

#endif /* AUGNET_WRITER_H */
