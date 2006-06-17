/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
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

struct aug_handlers {
    int (*setinitial_)(void*, const char*);
    int (*setfield_)(void*, const char*, const char*);
    int (*setcsize_)(void*, size_t);
    int (*cdata_)(void*, const void*, size_t);
    int (*end_)(void*, int);
};

#endif /* AUGNET_TYPES_H */
