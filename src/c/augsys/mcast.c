/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/mcast.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if ENABLE_MULTICAST
# if !defined(_WIN32)
#  include "augsys/posix/mcast.c"
# else /* _WIN32 */
#  include "augsys/win32/mcast.c"
# endif /* _WIN32 */
#endif /* ENABLE_MULTICAST */
