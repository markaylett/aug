/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGSERV_BUILD
#include "augserv/control.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augserv/posix/control.c"
#else /* _WIN32 */
# include "augserv/win32/control.c"
#endif /* _WIN32 */
