/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERV_LOG_H
#define AUGSERV_LOG_H

#include "augserv/config.h"

#include "augtypes.h"

/**
 * Re-direct standard file handles to specified log.
 */

AUGSERV_API aug_result
aug_openlog(const char* path);

AUGSERV_API aug_result
aug_setservlogger(const char* sname);

#endif /* AUGSERV_LOG_H */
