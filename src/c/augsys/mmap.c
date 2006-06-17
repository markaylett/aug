/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/mmap.h"

static const char rcsid[] = "$Id:$";

#if !defined(_WIN32)
# include "augsys/posix/mmap.c"
#else /* _WIN32 */
# include "augsys/win32/mmap.c"
#endif /* _WIN32 */
