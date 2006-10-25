/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/mplexer.h"

static const char rcsid[] = "$Id$";

#if !defined(_WIN32)
# include "augsys/posix/mplexer.c"
#else /* _WIN32 */
# include "augsys/win32/mplexer.c"
#endif /* _WIN32 */
