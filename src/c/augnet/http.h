/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_HTTP_H
#define AUGNET_HTTP_H

#include "augnet/config.h"

#include "augsys/types.h"

#include "augobj.h"

struct aug_httphandler {
    int (*initial_)(aug_object*, const char*);
    int (*field_)(aug_object*, const char*, const char*);
    int (*csize_)(aug_object*, unsigned);
    int (*cdata_)(aug_object*, const void*, unsigned);
    int (*end_)(aug_object*, int);
};

typedef struct aug_httpparser_* aug_httpparser_t;

/**
   If aug_createhttpparser() succeeds, aug_releaseobject() will be called from
   aug_destroyhttpparser().
*/

AUGNET_API aug_httpparser_t
aug_createhttpparser(unsigned size, const struct aug_httphandler* handler,
                     aug_object* ob);

AUGNET_API int
aug_destroyhttpparser(aug_httpparser_t parser);

AUGNET_API int
aug_appendhttp(aug_httpparser_t parser, const char*, unsigned size);

AUGNET_API int
aug_finishhttp(aug_httpparser_t parser);

#endif /* AUGNET_HTTP_H */
