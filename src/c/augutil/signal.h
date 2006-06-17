/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_SIGNAL_H
#define AUGUTIL_SIGNAL_H

#include "augutil/config.h"

#define AUG_SIGNONE   0
#define AUG_SIGRECONF 1
#define AUG_SIGSTATUS 2
#define AUG_SIGSTOP   3

/* Base value for user-defined signals. */

#define AUG_SIGUSER   4

typedef unsigned char aug_signal_t;

AUGUTIL_API aug_signal_t
aug_tosignal(int i);

AUGUTIL_API int
aug_readsignal(int fd, aug_signal_t* sig);

AUGUTIL_API int
aug_writesignal(int fd, aug_signal_t sig);

#endif /* AUGUTIL_SIGNAL_H */
