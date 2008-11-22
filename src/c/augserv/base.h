/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERV_BASE_H
#define AUGSERV_BASE_H

#include "augserv/config.h"

#include "augsys/types.h"

#include "augtypes.h"

struct aug_serv;

#if defined(AUGSERV_BUILD)
AUG_EXTERNC void
aug_setserv_(const struct aug_serv* serv);
#endif /* AUGSERV_BUILD */

AUGSERV_API const char*
aug_getservopt(int opt);

AUGSERV_API aug_result
aug_readservconf(const char* conffile, aug_bool batch, aug_bool daemon);

AUGSERV_API aug_result
aug_initserv(void);

AUGSERV_API aug_result
aug_runserv(void);

AUGSERV_API void
aug_termserv(void);

/**
 * Read-end of event pipe.
 */

AUGSERV_API aug_md
aug_eventrd(void);

/**
 * Write-end of event pipe.
 */

AUGSERV_API aug_md
aug_eventwr(void);

#endif /* AUGSERV_BASE_H */
