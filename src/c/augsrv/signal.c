/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/signal.h"

#if !defined(_WIN32)
# include "augsrv/posix/signal.c"
#else /* _WIN32 */
# include "augsrv/win32/signal.c"
#endif /* _WIN32 */
