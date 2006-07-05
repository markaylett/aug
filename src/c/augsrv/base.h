/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_BASE_H
#define AUGSRV_BASE_H

#include "augsrv/config.h"

struct aug_service;

#if defined(AUGSRV_BUILD)
AUGSRV_EXTERN void
aug_setservice_(const struct aug_service* service);

AUGSRV_EXTERN void
aug_seteventpipe_(int fds[2]);
#endif /* AUGSRV_BUILD */

AUGSRV_API const struct aug_service*
aug_service(void);

AUGSRV_API int
aug_eventin(void);

AUGSRV_API int
aug_eventout(void);

#endif /* AUGSRV_BASE_H */
