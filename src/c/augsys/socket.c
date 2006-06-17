/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/socket.h"

#if !defined(_WIN32)
# include "augsys/posix/socket.c"
#else /* _WIN32 */
# include "augsys/win32/socket.c"
#endif /* _WIN32 */
