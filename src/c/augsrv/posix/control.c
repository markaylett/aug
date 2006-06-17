/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrv/options.h"

#include "augsys/defs.h"
#include "augsys/errno.h"
#include "augsys/log.h"
#include "augsys/string.h"

#include <sys/types.h>
#include <fcntl.h>         /* struct flock */
#include <signal.h>        /* kill() */
#include <strings.h>       /* bzero() */
#include <unistd.h>

#define VERIFYCLOSE_(x) \
    AUG_VERIFY(close(x), "close() failed")

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
send_(int fd, pid_t pid, aug_sig_t sig)
{
    struct flock fl;

    switch (sig) {
    case AUG_SIGRECONF:
        if (-1 == kill(pid, SIGHUP))
            return -1;
        break;
    case AUG_SIGSTATUS:
        if (-1 == kill(pid, SIGUSR1))
            return -1;
        break;
    case AUG_SIGSTOP:
        if (-1 == kill(pid, SIGTERM))
            return -1;

        /* Wait for daemon process to release lock. */

        if (-1 == flock_(&fl, fd, F_SETLKW, F_RDLCK))
            return -1;

        /* The lock has been obtained; daemon process must have stopped. */

        break;
    default:

        /* Invalid command. */

        errno = EINVAL;
        return -1;
    }
    return 0;
}

AUGSRV_API int
aug_start(const struct aug_service* service)
{
    errno = EINVAL;
    return -1;
}

AUGSRV_API int
aug_control(const struct aug_service* service, aug_sig_t sig)
{
    const char* path;
    struct flock fl;
    int fd, ret = -1;

    if (!(path = (*service->getopt_)(service->arg_, AUG_OPTPIDFILE)))
        return -1;

    /* Check for existence of file. */

    if (-1 == access(path, F_OK))
        return AUG_ENOTEXISTS;

	if (-1 == (fd = open(path, O_RDONLY)))
        return -1;

    /* Attempt to obtain shared lock. */

    if (-1 == flock_(&fl, fd, F_SETLK, F_RDLCK)) {

        /* As expected, the daemon process has an exclusive lock on the pid
           file.  Use F_GETLK to obtain the pid of the daemon process. */

        if (-1 == flock_(&fl, fd, F_GETLK, F_RDLCK))
            goto done;

        /* Although a lock-manager allows locking over NFS, the returned pid
           is probably zero because it could be the pid of a process on
           another node. */

        if (0 == fl.l_pid) {
            aug_warn("lockfile stored on NFS mounted filesystem");
            errno = EINVAL;
            return -1;
        }

        if (-1 == send_(fd, fl.l_pid, sig))
            goto done;

        ret = 0;

    } else {

        /* No lock was obtained, therefore, the daemon process cannot be
           running. */

        if (-1 == unlink(path))
            goto done;

        ret = AUG_ENOTEXISTS;
    }

 done:
    VERIFYCLOSE_(fd);
    return ret;
}

AUGSRV_API int
aug_install(const struct aug_service* service)
{
    return 0;
}

AUGSRV_API int
aug_uninstall(const struct aug_service* service)
{
    return 0;
}
