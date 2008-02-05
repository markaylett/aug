/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/tls_.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if ENABLE_THREADS
# if !defined(_WIN32)
#  include "augctx/posix/tls.c"
# else /* _WIN32 */
#  include "augctx/win32/tls.c"
# endif /* _WIN32 */
#endif /* ENABLE_THREADS */
