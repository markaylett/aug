/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_HTTP_H
#define AUGNET_HTTP_H

/**
 * @file augnet/http.h
 *
 * HTTP parser.
 */

#include "augnet/config.h"

#include "augext/http.h"
#include "augext/mpool.h"

typedef struct aug_httpparser_* aug_httpparser_t;

/**
 * If aug_createhttpparser() succeeds, aug_release() will be called from
 * aug_destroyhttpparser().
 */

AUGNET_API aug_httpparser_t
aug_createhttpparser(aug_mpool* mpool, aug_httphandler* handler,
                     unsigned size);

AUGNET_API void
aug_destroyhttpparser(aug_httpparser_t parser);

AUGNET_API aug_result
aug_appendhttp(aug_httpparser_t parser, const char*, unsigned size);

AUGNET_API aug_result
aug_finishhttp(aug_httpparser_t parser);

#endif /* AUGNET_HTTP_H */
