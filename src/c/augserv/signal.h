/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERV_SIGNAL_H
#define AUGSERV_SIGNAL_H

#include "augserv/config.h"

#include "augtypes.h"

AUGSERV_API aug_result
aug_setsighandler(void (*handler)(int));

AUGSERV_API aug_result
aug_blocksignals(void);

AUGSERV_API aug_result
aug_unblocksignals(void);

#endif /* AUGSERV_SIGNAL_H */
