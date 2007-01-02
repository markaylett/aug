/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_TYPES_H
#define AUGNET_TYPES_H

#include "augsys/types.h"

struct iovec;

struct aug_buf {
    struct iovec* iov_;
    size_t size_;
};

#endif /* AUGNET_TYPES_H */
