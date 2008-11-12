/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/signal.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augsrv/posix/signal.c"
#else /* _WIN32 */
# include "augsrv/win32/signal.c"
#endif /* _WIN32 */
