/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_FIX_H
#define AUGNET_FIX_H

#include "augnet/config.h"
#include "augnet/types.h"

#define AUG_FIXVERLEN  7  /* FIX.x.y */

typedef char aug_fixver_t[AUG_FIXVERLEN + 1];
typedef unsigned aug_fixtag_t;

struct aug_fixfield_ {
    aug_fixtag_t tag_;
    const char* value_;
    aug_len_t size_;
};

struct aug_fixstd_ {
    aug_fixver_t fixver_;
    const char* body_;
    size_t size_;
};

typedef void (*aug_fixcb_t)(aug_object*, const char*, size_t);

typedef struct aug_fixstream_* aug_fixstream_t;

/**
   If aug_createfixstream() succeeds, aug_decref() will be called from
   aug_destroyfixstream().
*/

AUGNET_API aug_fixstream_t
aug_createfixstream(size_t size, aug_fixcb_t cb, aug_object* ob);

AUGNET_API int
aug_destroyfixstream(aug_fixstream_t stream);

AUGNET_API ssize_t
aug_readfix(aug_fixstream_t stream, int fd, size_t size);

AUGNET_API int
aug_finishfix(aug_fixstream_t stream);

AUGNET_API int
aug_checkfix(struct aug_fixstd_* fixstd, const char* buf, size_t size);

AUGNET_API aug_len_t
aug_checksum(const char* buf, size_t size);

AUGNET_API ssize_t
aug_fixfield(struct aug_fixfield_* field, const char* buf, size_t size);

#endif /* AUGNET_FIX_H */
