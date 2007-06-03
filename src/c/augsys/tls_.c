/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/tls_.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#if ENABLE_THREADS
# if !defined(_WIN32)
#  include "augsys/posix/tls.c"
# else /* _WIN32 */
#  include "augsys/win32/tls.c"
# endif /* _WIN32 */
#endif /* ENABLE_THREADS */
