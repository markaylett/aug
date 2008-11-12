/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_LOG_H
#define AUGSRV_LOG_H

#include "augsrv/config.h"

#include "augtypes.h"

/**
 * Re-direct standard file handles to specified log.
 */

AUGSRV_API aug_result
aug_openlog(const char* path);

AUGSRV_API aug_result
aug_setsrvlogger(const char* sname);

#endif /* AUGSRV_LOG_H */
