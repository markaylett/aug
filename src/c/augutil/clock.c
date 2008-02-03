/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/clock.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augutil/posix/clock.c"
#else /* _WIN32 */
# include "augutil/win32/clock.c"
#endif /* _WIN32 */
