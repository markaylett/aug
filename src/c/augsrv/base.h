/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_BASE_H
#define AUGSRV_BASE_H

#include "augsrv/config.h"
#include "augsrv/types.h"

#if defined(AUGSRV_BUILD)
AUG_EXTERNC void
aug_setserver_(const struct aug_server* server, void* arg);
#endif /* AUGSRV_BUILD */

AUGSRV_API const char*
aug_getserveropt(enum aug_option opt);

AUGSRV_API int
aug_readserverconf(const char* conffile, int prompt, int daemon);

AUGSRV_API int
aug_initserver(void);

AUGSRV_API int
aug_runserver(void);

AUGSRV_API void
aug_termserver(void);

AUGSRV_API int
aug_eventrd(void);

AUGSRV_API int
aug_eventwr(void);

#endif /* AUGSRV_BASE_H */
