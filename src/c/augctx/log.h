/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_LOG_H
#define AUGCTX_LOG_H

#include "augctx/config.h"

#include "augext/log.h"

/**
 * The standard logger.
 *
 * @return An interface to the standard logger.
 */

AUGCTX_API aug_log*
aug_getstdlog(void);

#endif /* AUGCTX_LOG_H */
