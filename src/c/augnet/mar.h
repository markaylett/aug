/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
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

#include "augsys/types.h"

#include "augext/mar.h"
#include "augext/mpool.h"

#include "augtypes.h"

typedef struct aug_marparser_* aug_marparser_t;

AUGNET_API aug_marparser_t
aug_createmarparser(aug_mpool* mpool, aug_marpool* marpool, unsigned size);

AUGNET_API void
aug_destroymarparser(aug_marparser_t parser);

AUGNET_API aug_result
aug_appendmar(aug_marparser_t parser, const char*, unsigned size);

AUGNET_API aug_result
aug_finishmar(aug_marparser_t parser);

#endif /* AUGNET_MAR_H */
