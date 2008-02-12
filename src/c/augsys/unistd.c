/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/unistd.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

# if !defined(_WIN32)
#  include "augsys/posix/unistd.c"
# else /* _WIN32 */
#  include "augsys/win32/unistd.c"
# endif /* _WIN32 */
