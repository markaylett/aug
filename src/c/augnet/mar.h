/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_MAR_H
#define AUGNET_MAR_H

#include "augnet/config.h"

#include "augmar/types.h"
#include "augsys/types.h"

struct aug_var;

struct aug_marhandler {
    aug_mar_t (*create_)(const struct aug_var*, const char*);
    int (*message_)(const struct aug_var*, const char*, aug_mar_t);
};

typedef struct aug_marparser_* aug_marparser_t;

/**
   If aug_createmarparser() succeeds, aug_destroyvar() will be called from
   aug_destroymarparser().
*/

AUGNET_API aug_marparser_t
aug_createmarparser(unsigned size, const struct aug_marhandler* handler,
                    const struct aug_var* var);

AUGNET_API int
aug_destroymarparser(aug_marparser_t parser);

AUGNET_API int
aug_parsemar(aug_marparser_t parser, const char*, unsigned size);

AUGNET_API int
aug_endmar(aug_marparser_t parser);

#endif /* AUGNET_MAR_H */
