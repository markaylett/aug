/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/tls_.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#if !defined(_MT)

# include <errno.h>

AUGSYS_EXTERN int
aug_createtlskey_(aug_tlskey_t* tlskey)
{
    errno = ENOSYS;
    return -1;
}

AUGSYS_EXTERN int
aug_destroytlskey_(aug_tlskey_t tlskey)
{
    errno = ENOSYS;
    return -1;
}

AUGSYS_EXTERN int
aug_gettlsvalue_(aug_tlskey_t tlskey, void** value)
{
    errno = ENOSYS;
    return -1;
}

AUGSYS_EXTERN int
aug_settlsvalue_(aug_tlskey_t tlskey, void* value)
{
    errno = ENOSYS;
    return -1;
}

#else /* _MT */
# if !defined(_WIN32)
#  include "augsys/posix/tls.c"
# else /* _WIN32 */
#  include "augsys/win32/tls.c"
# endif /* _WIN32 */
#endif /* _MT */
