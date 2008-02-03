/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augaspp/buffer.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutilpp/object.hpp"

#include <string>

using namespace aug;
using namespace aug;
using namespace std;

buffer::~buffer() AUG_NOTHROW
{
}

buffer::buffer(size_t size)
    : blob_(size),
      reuse_(true),
      size_(0)
{
}

void
buffer::append(blobref ref)
{
    appendwriter(writer_, ref);
    reuse_ = false;

    size_ += blobsize(ref);
}

void
buffer::append(const void* buf, size_t len)
{
    if (reuse_) {

        // Exclusively using internal blob.

        if (empty()) {

            // But the writer is empty so it needs to be appended.

            appendwriter(writer_, blob_);
        }

        blob_.append(buf, len);

    } else {

        // External blobs are being written so the buffer will need to be
        // converted to a blob.

        smartob<aug_blob> blob(createblob(buf, len));
        appendwriter(writer_, blob);
    }

    size_ += len;
}

size_t
buffer::writesome(fdref ref)
{
    size_t size(aug::writesome(writer_, ref));

    // Once appended, external blobs must not mutate.  AUG_MIN is used here as
    // a defensive measure.

    size_ -= AUG_MIN(size_, size);

    if (writer_.empty()) {

        // Safe to use internal blob: all external blobs have been written.

        reuse_ = true;
    }

    return size;
}
