/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_BASE64_H
#define AUGNET_BASE64_H

/**
 * @file augnet/base64.h
 *
 * Base 64 encoder and decoder.
 */

#include "augnet/config.h"

#include "augsys/types.h"

#include "augext/mpool.h"

#include "augabi.h"
#include "augtypes.h"

typedef aug_result (*aug_base64cb_t)(aug_object*, const char*, size_t);

typedef struct aug_base64_* aug_base64_t;

enum aug_base64mode {
	AUG_DECODE64,
	AUG_ENCODE64
};

AUGNET_API aug_base64_t
aug_createbase64(aug_mpool* mpool, enum aug_base64mode mode,
                 aug_base64cb_t cb, aug_object* ob);

AUGNET_API void
aug_destroybase64(aug_base64_t base64);

AUGNET_API aug_result
aug_appendbase64(aug_base64_t base64, const char* src, size_t len);

AUGNET_API aug_result
aug_finishbase64(aug_base64_t base64);

#endif /* AUGNET_BASE64_H */
