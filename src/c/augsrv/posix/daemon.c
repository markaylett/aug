/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrv/options.h"
#include "augsrv/types.h"

#include "augutil/log.h"

#include "augsys/defs.h"
#include "augsys/errno.h"
#include "augsys/string.h"

#include <assert.h>
#include <alloca.h>
#include <fcntl.h>         /* struct flock */
#include <math.h>          /* log10() */
#include <stdio.h>         /* snprintf() */
#include <strings.h>       /* bzero() */
#include <unistd.h>        /* getdtablesize() */
#include <sys/types.h>     /* umask() */
#include <sys/stat.h>      /* umask() */

#define VERIFYCLOSE_(x) \
    AUG_VERIFY(close(x), "close() failed")

static int
closeall_(int next)
{
    int limit = getdtablesize();
    for (; next <= limit; ++next)
        close(next);

    return 0;
}

static int
daemonise_(void)
{
    switch (fork()) {
    case 0:
        break;
    case -1:
        return -1;
    default:
        /* Use system version of exit to avoid flushing standard
           streams. */
        _exit(0);
    }

    /* Detach from controlling terminal by making process a session
       leader. */

    if (-1 == setsid())
        return -1;

    /* Forking again ensures that the daemon process is not a session leader,
       and therefore cannot regain access to a controlling terminal. */

    switch (fork()) {
    case 0:
        break;
    case -1:
        return -1;
    default:
        /* Use system version of exit to avoid flushing standard streams. */
        _exit(0);
    }

    /* Restrict file creation mode. */

    if (-1 == umask(0027))
        return -1;

    /* Leave both the standard file descriptors (3), and the signal pipe
       descriptors (2) open. */

    if (-1 == closeall_(3 + 2))
        return -1;

    return 0;
}

size_t
digits_(unsigned long n)
{
    return 0 == n ? 1 : ((int)log10((double)n)) + 1;
}

static int
writepid_(int fd)
{
    pid_t pid = getpid();
    size_t len = digits_(pid) + 1;
    char* str = alloca(sizeof(char) * (len + 1));
    if (!str)
        return -1;

    /* Buffer is always terminated with null byte. */

    if (len != (snprintf(str, len + 1, "%ld\n", (long)pid)))
        return -1;

    if (len != write(fd, str, len))
        return -1;

    if (-1 == fsync(fd))
        return -1;

    return 0;
}

static int
flock_(struct flock* fl, int fd, int cmd, int type)
{
    bzero(fl, sizeof(*fl));

    fl->l_type = type;
    fl->l_whence = SEEK_SET;
    fl->l_start = 0;
    fl->l_len = 0;

    return fcntl(fd, cmd, fl);
}

static int
lockfile_(const char* path)
{
    struct flock fl;
    int fd = open(path, O_CREAT | O_WRONLY, 0640);
    if (-1 == fd)
        return -1;

    /* Attempt to obtain exclusive lock. */

    if (-1 == flock_(&fl, fd, F_SETLK, F_WRLCK)) {

        if (EAGAIN != errno)
            goto fail;

        /* EAGAIN indicates that another process has locked the file. */

        VERIFYCLOSE_(fd);
        return AUG_EEXISTS;
    }

    /* Truncate any existing pid value. */

    if (-1 == ftruncate(fd, 0))
        goto fail;

    if (-1 == writepid_(fd))
        goto fail;

    /* Success: do not close the file - this would release the lock. */

    return 0;

 fail:
    VERIFYCLOSE_(fd);
    return -1;
}

static int
closein_(void)
{
    int fd = open("/dev/null", O_RDONLY), ret = -1;
	if (-1 == fd)
        return -1;

    if (-1 != dup2(fd, STDIN_FILENO))
        ret = 0;

    VERIFYCLOSE_(fd);
    return ret;
}

AUGSRV_API int
aug_daemonise(const struct aug_service* service)
{
    const char* path = (*service->getopt_)(service->arg_, AUG_OPTPIDFILE);

    if (!path || -1 == daemonise_())
        return -1;

    switch (lockfile_(path)) {
    case -1:
        return -1;
    case AUG_EEXISTS:
        return AUG_EEXISTS;
    }

    if (-1 == closein_())
        return -1;

    return -1 == (*service->init_)(service->arg_)
        || -1 == (*service->run_)(service->arg_)
        ? -1 : 0;
}
