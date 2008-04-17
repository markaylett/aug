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

#if !defined(STDOUT_FILENO)
# define STDOUT_FILENO 1
#endif /* !STDOUT_FILENO */

#if !defined(STDERR_FILENO)
# define STDERR_FILENO 2
#endif /* !STDERR_FILENO */

static int
redirectout_(int fd)
{
    int old, ret = -1;

#if !defined(_WIN32)
    if (EOF == fflush(NULL))
        return -1;
#else /* _WIN32 */
    fflush(NULL);
#endif /*_WIN32 */

    /* Duplicate stdout descriptor so that it can be restored on failure. */

    if (-1 == (old = dup(STDOUT_FILENO))) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    /* Assumption: If dup2 fails for any reason, the original descriptor's
       state will remain unchanged. */

    if (-1 == dup2(fd, STDOUT_FILENO)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        goto done;
    }

    if (-1 == dup2(fd, STDERR_FILENO)) {

        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);

        /* Restore the original descriptor. */

        if (-1 == dup2(old, STDOUT_FILENO))
            aug_ctxerror(aug_tlx, "dup2() failed");
        goto done;
    }

    /* Success */

    ret = 0;

 done:
    if (-1 == close(old))
        aug_ctxerror(aug_tlx, "close() failed");
    return ret;
}

AUGSRV_API int
aug_openlog(const char* path)
{
    int fd, ret = 0;

    if (-1 == (fd = open(path, O_APPEND | O_CREAT | O_WRONLY, 0640))) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    if (-1 == redirectout_(fd))
        ret = -1;

    if (-1 == close(fd))
        aug_ctxerror(aug_tlx, "close() failed");
    return ret;
}
