/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define AUGSERV_BUILD
#include "augserv/log.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augserv/posix/log.c"
#else /* _WIN32 */
# include "augserv/win32/log.c"
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

AUGSERV_API aug_result
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
