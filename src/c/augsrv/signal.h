/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_SIGNAL_H
#define AUGSRV_SIGNAL_H

#include "augsrv/config.h"

AUGSRV_API int
aug_signalhandler(void (*handler)(int));

AUGSRV_API int
aug_blocksignals(void);

AUGSRV_API int
aug_unblocksignals(void);

#endif /* AUGSRV_SIGNAL_H */
