/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrv/base.h"
#include "augsrv/options.h"

#include "augutil/log.h"

#include "augsys/defs.h"
#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/string.h"

#include <assert.h>
#if HAVE_ALLOCA_H
# include <alloca.h>
#endif /* HAVE_ALLOCA_H */
#include <fcntl.h>         /* struct flock */
#include <math.h>          /* log10() */
#include <stdio.h>         /* snprintf() */
#include <strings.h>       /* bzero() */
#include <unistd.h>        /* getdtablesize() */
#include <sys/types.h>     /* umask() */
#include <sys/stat.h>      /* umask() */

#define VERIFYCLOSE_(x) \
    AUG_PERROR(close(x), "close() failed")

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
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    default:
        /* Use system version of exit to avoid flushing standard
           streams. */
        _exit(0);
    }

    /* Detach from controlling terminal by making process a session
       leader. */

    if (-1 == setsid()) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    /* Forking again ensures that the daemon process is not a session leader,
       and therefore cannot regain access to a controlling terminal. */

    switch (fork()) {
    case 0:
        break;
    case -1:
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    default:
        /* Use system version of exit to avoid flushing standard streams. */
        _exit(0);
    }

    /* Restrict file creation mode. */
    /* No need to check the return value: it is the previous value of the file
       mode mask. This function is always successful */
    umask(0027);

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
    if (!str) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return -1;
    }

    /* Buffer is always terminated with null byte. */

    if (len != (snprintf(str, len + 1, "%ld\n", (long)pid))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EIO,
                       AUG_MSG("pid formatting failed"));
        return -1;
    }

    if (len != write(fd, str, len)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    if (-1 == fsync(fd)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

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

    if (-1 == fcntl(fd, cmd, fl)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static int
lockfile_(const char* path)
{
    struct flock fl;
    int fd = open(path, O_CREAT | O_WRONLY, 0640);
    if (-1 == fd) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    /* Attempt to obtain exclusive lock. */

    if (-1 == flock_(&fl, fd, F_SETLK, F_WRLCK)) {

        if (!aug_iserrinfo(NULL, AUG_SRCPOSIX, EAGAIN))
            goto fail;

        /* EAGAIN indicates that another process has locked the file. */

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                       AUG_MSG("pidfile still in use: %s"), path);

        VERIFYCLOSE_(fd);
        return -1;
    }

    /* Truncate any existing pid value. */

    if (-1 == ftruncate(fd, 0)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        goto fail;
    }

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
	if (-1 == fd) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    if (-1 == dup2(fd, STDIN_FILENO))
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
    else
        ret = 0;

    VERIFYCLOSE_(fd);
    return ret;
}

AUGSRV_API int
aug_daemonise(void)
{
    const char* pidfile;
    int ret;

    if (!(pidfile = aug_getserveropt(AUG_OPTPIDFILE))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTPIDFILE' not set"));
        return -1;
    }

    if (-1 == daemonise_()
        || -1 == lockfile_(pidfile)
        || -1 == closein_()
        || -1 == aug_initserver())
        return -1;

    ret = aug_runserver();
    aug_termserver();
    return ret;
}
