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
#define AUGASPP_BUILD
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

buffer::buffer(mpoolref mpool, size_t size)
    : writer_(mpool),
      blob_(size),
      reuse_(true),
      size_(0)
{
}

void
buffer::append(blobref ref)
{
    appendwriter(writer_, ref);
    reuse_ = false;

    size_ += getblobsize(ref);
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

        smartob<aug_blob> blob(createblob(getmpool(aug_tlx), buf, len));
        appendwriter(writer_, blob);
    }

    size_ += len;
}

size_t
buffer::writesome(streamref ref)
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
