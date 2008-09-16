/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/log.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augsrv/posix/log.c"
#else /* _WIN32 */
# include "augsrv/win32/log.c"
#endif /* _WIN32 */

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/string.h"

#include <errno.h>
#include <stdio.h>          /* fflush() */

#include <fcntl.h>
#if !defined(_WIN32)
# include <unistd.h>        /* dup() */
#else /* _WIN32 */
# include <io.h>            /* dup() */
#endif /* _WIN32 */

#if !defined(STDOUT_FILENO)
# define STDOUT_FILENO 1
#endif /* !STDOUT_FILENO */

#if !defined(STDERR_FILENO)
# define STDERR_FILENO 2
#endif /* !STDERR_FILENO */

static aug_result
redirectout_(int fd)
{
    int old;
    aug_result result;

#if !defined(_WIN32)
    if (EOF == fflush(NULL))
        return AUG_FAILERROR;
#else /* _WIN32 */
    fflush(NULL);
#endif /*_WIN32 */

    /* Duplicate stdout descriptor so that it can be restored on failure. */

    if (-1 == (old = dup(STDOUT_FILENO)))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    /* Assumption: If dup2 fails for any reason, the original descriptor's
       state will remain unchanged. */

    if (-1 == dup2(fd, STDOUT_FILENO)) {
        result = aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        goto done;
    }

    if (-1 == dup2(fd, STDERR_FILENO)) {

        result = aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

        /* Restore the original descriptor. */

        if (-1 == dup2(old, STDOUT_FILENO))
            aug_ctxerror(aug_tlx, "dup2() failed");
        goto done;
    }

    /* Success */

    result = AUG_SUCCESS;

 done:
    if (-1 == close(old))
        aug_ctxerror(aug_tlx, "close() failed");

    return result;
}

AUGSRV_API aug_result
aug_openlog(const char* path)
{
    int fd;
    aug_result result;

    if (-1 == (fd = open(path, O_APPEND | O_CREAT | O_WRONLY, 0640)))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    result = redirectout_(fd);

    if (-1 == close(fd))
        aug_ctxerror(aug_tlx, "close() failed");

    return result;
}
