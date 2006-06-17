/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_SIGNAL_H
#define AUGSRV_SIGNAL_H

#include "augsrv/config.h"
#include "augsrv/types.h" /* enum aug_signal */

AUGSRV_API int
aug_sigactions(void (*handler)(int));

AUGSRV_API int
aug_sigblock(void);

AUGSRV_API int
aug_sigunblock(void);

AUGSRV_API enum aug_signal
aug_tosignal(int i);

AUGSRV_API int
aug_readsig(enum aug_signal* sig);

AUGSRV_API int
aug_writesig(enum aug_signal sig);

#endif /* AUGSRV_SIGNAL_H */
