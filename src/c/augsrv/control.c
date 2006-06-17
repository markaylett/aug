/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/control.h"

#if !defined(_WIN32)
# include "augsrv/posix/control.c"
#else /* _WIN32 */
# include "augsrv/win32/control.c"
#endif /* _WIN32 */
