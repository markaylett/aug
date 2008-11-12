/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/control.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augsrv/posix/control.c"
#else /* _WIN32 */
# include "augsrv/win32/control.c"
#endif /* _WIN32 */
