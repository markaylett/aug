/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_HTTP_H
#define AUGNET_HTTP_H

#include "augnet/config.h"

#include "augsys/types.h"

struct aug_var;

struct aug_httphandler {
    int (*initial_)(const struct aug_var*, const char*);
    int (*field_)(const struct aug_var*, const char*, const char*);
    int (*csize_)(const struct aug_var*, unsigned);
    int (*cdata_)(const struct aug_var*, const void*, unsigned);
    int (*end_)(const struct aug_var*, int);
};

typedef struct aug_httpparser_* aug_httpparser_t;

/**
   If aug_createhttpparser() succeeds, aug_destroyvar() will be called from
   aug_destroyhttpparser().
*/

AUGNET_API aug_httpparser_t
aug_createhttpparser(unsigned size, const struct aug_httphandler* handler,
                     const struct aug_var* var);

AUGNET_API int
aug_destroyhttpparser(aug_httpparser_t parser);

AUGNET_API int
aug_parsehttp(aug_httpparser_t parser, const char*, unsigned size);

AUGNET_API int
aug_endhttp(aug_httpparser_t parser);

#endif /* AUGNET_HTTP_H */
