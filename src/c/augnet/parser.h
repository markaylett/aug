/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_PARSER_H
#define AUGNET_PARSER_H

#include "augnet/config.h"
#include "augsys/types.h"

struct aug_handlers {
    int (*setinitial_)(void*, const char*);
    int (*setfield_)(void*, const char*, const char*);
    int (*setcsize_)(void*, size_t);
    int (*cdata_)(void*, const void*, size_t);
    int (*end_)(void*, int);
};

typedef struct aug_parser_* aug_parser_t;

AUGNET_API aug_parser_t
aug_createparser(size_t size, const struct aug_handlers* handlers, void* arg);

AUGNET_API int
aug_freeparser(aug_parser_t parser);

AUGNET_API int
aug_parse(aug_parser_t parser, const char* buf, size_t size);

AUGNET_API int
aug_parseend(aug_parser_t parser);

#endif /* AUGNET_PARSER_H */
