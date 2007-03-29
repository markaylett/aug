/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_BASE64_H
#define AUGUTIL_BASE64_H

#include "augutil/config.h"

#include "augsys/types.h"

struct aug_md5context {
    uint32_t buf[4];
    uint32_t bits[2];
    unsigned char in[64];
};

AUGUTIL_API void
aug_initmd5(struct aug_md5context* ctx);

AUGUTIL_API void
aug_updatemd5(struct aug_md5context* ctx, const unsigned char* buf,
              unsigned len);

AUGUTIL_API void
aug_finalmd5(unsigned char digest[16], struct aug_md5context* ctx);

AUGUTIL_API void
aug_transformmd5(uint32_t buf[4], const uint32_t in[16]);

#endif /* AUGUTIL_MD5_H */
