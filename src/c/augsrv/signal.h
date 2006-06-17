/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_SIGNAL_H
#define AUGSRV_SIGNAL_H

#include "augsrv/config.h"
#include "augsrv/types.h" /* enum aug_sig_t */

AUGSRV_API int
aug_sigactions(void (*handler)(int));

AUGSRV_API int
aug_sigblock(void);

AUGSRV_API int
aug_sigunblock(void);

AUGSRV_API aug_sig_t
aug_tosignal(int i);

AUGSRV_API int
aug_readsig(aug_sig_t* sig);

AUGSRV_API int
aug_writesig(aug_sig_t sig);

#endif /* AUGSRV_SIGNAL_H */
