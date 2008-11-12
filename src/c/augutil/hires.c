/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/hires.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augutil/posix/hires.c"
#else /* _WIN32 */
# include "augutil/win32/hires.c"
#endif /* _WIN32 */
