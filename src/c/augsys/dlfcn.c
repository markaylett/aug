/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/dlfcn.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augsys/posix/dlfcn.c"
#else /* _WIN32 */
# include "augsys/win32/dlfcn.c"
#endif /* _WIN32 */
