/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_LOG_H
#define AUGSRV_LOG_H

#include "augsrv/config.h"

AUGSRV_API int
aug_openlog(const char* path);

AUGSRV_API int
aug_setsrvlogger(const char* sname);

AUGSRV_API int
aug_unsetsrvlogger(void);

#endif /* AUGSRV_LOG_H */
