/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/dlfcn.h"

static const char rcsid[] = "$Id: $";

#if !defined(_WIN32)
# include "augsys/posix/dlfcn.c"
#else /* _WIN32 */
# include "augsys/win32/dlfcn.c"
#endif /* _WIN32 */
