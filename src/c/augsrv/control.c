/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#undef __STRICT_ANSI__ /* bzero() */
#define AUGSRV_BUILD
#include "augsrv/control.h"

static const char rcsid[] = "$Id:$";

#if !defined(_WIN32)
# include "augsrv/posix/control.c"
#else /* _WIN32 */
# include "augsrv/win32/control.c"
#endif /* _WIN32 */
