/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_BASE_H
#define AUGSRV_BASE_H

#include "augsrv/config.h"
#include "augsrv/types.h"

#include "augsys/types.h"

#if defined(AUGSRV_BUILD)
AUG_EXTERNC void
aug_setservice_(const struct aug_service* service, void* arg);
#endif /* AUGSRV_BUILD */

AUGSRV_API const char*
aug_getserviceopt(int opt);

AUGSRV_API aug_result
aug_readserviceconf(const char* conffile, int batch, int daemon);

AUGSRV_API aug_result
aug_initservice(void);

AUGSRV_API aug_result
aug_runservice(void);

AUGSRV_API void
aug_termservice(void);

/**
 * Read-end of event pipe.
 */

AUGSRV_API aug_md
aug_eventrd(void);

/**
 * Write-end of event pipe.
 */

AUGSRV_API aug_md
aug_eventwr(void);

#endif /* AUGSRV_BASE_H */
