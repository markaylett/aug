/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/utility.h"

static const char rcsid[] = "$Id:$";

#include "augsys/uio.h"

AUGNET_API void
aug_addbuf(struct aug_buf* buf, size_t num)
{
    while (buf->size_ && (size_t)buf->iov_->iov_len <= num) {
        num -= (buf->iov_++)->iov_len;
        --buf->size_;
    }

    if (buf->size_ && num) {

        buf->iov_->iov_base = (char*)buf->iov_->iov_base + num;
        buf->iov_->iov_len -= (int)num;
        num = 0;
    }
}
