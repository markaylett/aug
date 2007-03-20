/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_BASE64_H
#define AUGNET_BASE64_H

#include "augnet/config.h"

#include "augsys/types.h"

struct aug_var;

typedef int (*aug_base64cb_t)(const struct aug_var*, const char*, size_t);

typedef struct aug_base64_* aug_base64_t;

enum aug_base64mode {
	AUG_DECODE64,
	AUG_ENCODE64
};

AUGNET_API aug_base64_t
aug_createbase64(enum aug_base64mode mode, aug_base64cb_t cb,
                 const struct aug_var* var);

AUGNET_API int
aug_destroybase64(aug_base64_t base64);

AUGNET_API int
aug_appendbase64(aug_base64_t base64, const char* src, size_t len);

AUGNET_API int
aug_endbase64(aug_base64_t base64);

#endif /* AUGNET_BASE64_H */
