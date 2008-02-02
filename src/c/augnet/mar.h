/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_MAR_H
#define AUGNET_MAR_H

/**
 * @file augnet/mar.h
 *
 * Meta ARchive parser.
 */

#include "augnet/config.h"

#include "augmar/types.h"

#include "augsys/types.h"

#include "augabi.h"

struct aug_marhandler {
    aug_mar_t (*create_)(aug_object*, const char*);
    int (*message_)(aug_object*, const char*, aug_mar_t);
};

typedef struct aug_marparser_* aug_marparser_t;

/**
 * If aug_createmarparser() succeeds, aug_release() will be called from
 * aug_destroymarparser().
 */

AUGNET_API aug_marparser_t
aug_createmarparser(unsigned size, const struct aug_marhandler* handler,
                    aug_object* ob);

AUGNET_API int
aug_destroymarparser(aug_marparser_t parser);

AUGNET_API int
aug_appendmar(aug_marparser_t parser, const char*, unsigned size);

AUGNET_API int
aug_finishmar(aug_marparser_t parser);

#endif /* AUGNET_MAR_H */
