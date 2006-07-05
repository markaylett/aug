/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#undef __STRICT_ANSI__ /* snprintf() */
#define AUGSRV_BUILD
#include "augsrv/daemon.h"

static const char rcsid[] = "$Id:$";

#if !defined(_WIN32)
# include "augsrv/posix/daemon.c"
#else /* _WIN32 */
# include "augsrv/win32/daemon.c"
#endif /* _WIN32 */
