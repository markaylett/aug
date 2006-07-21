/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/socket.h"

static const char rcsid[] = "$Id:$";

#if !defined(_WIN32)
# include "augsys/posix/socket.c"
#else /* _WIN32 */
# include "augsys/win32/socket.c"
#endif /* _WIN32 */

AUGSYS_API int
aug_setreuseaddr(int s, int on)
{
    return aug_setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
}

AUGSYS_API int
aug_getfamily(int s)
{
    struct aug_endpoint ep;
    if (!aug_getsockname(s, &ep))
        return -1;

    return ep.un_.family_;
}
