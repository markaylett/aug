/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augserv/base.h"
#include "augserv/options.h"
#include "augserv/types.h"

#include "augutil/log.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"
#include "augctx/string.h"

#include <assert.h>
#if HAVE_ALLOCA_H
# include <alloca.h>
#endif /* HAVE_ALLOCA_H */
#include <fcntl.h>          /* struct flock */
#include <math.h>           /* log10() */
#include <stdio.h>          /* snprintf() */
#include <strings.h>        /* bzero() */
#include <unistd.h>         /* getdtablesize() */
#include <sys/types.h>      /* umask() */
#include <sys/stat.h>       /* umask() */

static void
closeall_(int next)
{
    int limit = getdtablesize();
    for (; next <= limit; ++next)
        close(next);
}

static aug_result
daemonise_(void)
{
    switch (fork()) {
    case 0:
        break;
    case -1:
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
    default:
        /* Use system version of exit to avoid flushing standard
           streams. */
        _exit(0);
    }

    /* Detach from controlling terminal by making process a session
       leader. */

    if (-1 == setsid())
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    /* Forking again ensures that the daemon process is not a session leader,
       and therefore cannot regain access to a controlling terminal. */

    switch (fork()) {
    case 0:
        break;
    case -1:
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
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

    closeall_(3 + 2);
    return AUG_SUCCESS;
}

size_t
digits_(unsigned long n)
{
    return 0 == n ? 1 : ((int)log10((double)n)) + 1;
}

static aug_result
writepid_(int fd)
{
    pid_t pid = getpid();
    size_t len = digits_(pid) + 1; /* One for newline. */
    char* str = alloca(sizeof(char) * (len + 1));
    if (!str)
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, ENOMEM);

    /* Resulting buffer will _not_ be null terminated. */

    if (len != (snprintf(str, len, "%ld\n", (long)pid))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EFORMAT,
                       AUG_MSG("pid formatting failed"));
        return AUG_FAILERROR;
    }

    if (len != write(fd, str, len) || -1 == fsync(fd))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}

static aug_result
flock_(struct flock* fl, int fd, int cmd, int type)
{
    bzero(fl, sizeof(*fl));

    fl->l_type = type;
    fl->l_whence = SEEK_SET;
    fl->l_start = 0;
    fl->l_len = 0;

    if (-1 == fcntl(fd, cmd, fl))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}

static aug_result
lockfile_(const char* path)
{
    struct flock fl;
    int fd;
    aug_result result;

    if (-1 == (fd = open(path, O_CREAT | O_WRONLY, 0640)))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    /* Attempt to obtain exclusive lock. */

    if (AUG_ISFAIL(result = flock_(&fl, fd, F_SETLK, F_WRLCK))) {

        if (AUG_ISBLOCK(result)) {

            /* EWOULDBLOCK indicates that another process has locked the
               file. */

            aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EEXIST,
                           AUG_MSG("pidfile still in use: %s"), path);
        }
        goto fail;
    }

    /* Truncate any existing pid value. */

    if (-1 == ftruncate(fd, 0)) {
        result = aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        goto fail;
    }

    if (AUG_ISFAIL(result = writepid_(fd)))
        goto fail;

    /* Success: do not close the file - this would release the lock. */

    return AUG_SUCCESS;

 fail:
    close(fd);
    return result;
}

static aug_result
closein_(void)
{
    int fd;
    aug_result result;

	if (-1 == (fd = open("/dev/null", O_RDONLY)))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    if (-1 == dup2(fd, STDIN_FILENO))
        result = aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
    else
        result = AUG_SUCCESS;

    close(fd);
    return result;
}

AUGSERV_API aug_result
aug_daemonise(const struct aug_options* options)
{
    const char* pidfile;
    aug_result result;

    /* Install daemon logger prior to opening log file. */

    aug_setlog(aug_tlx, aug_getdaemonlog());

    aug_verify(aug_readservconf(AUG_CONFFILE(options), options->batch_,
                                AUG_TRUE));

    if (!(pidfile = aug_getservopt(AUG_OPTPIDFILE))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTPIDFILE' not set"));
        return AUG_FAILERROR;
    }

    if (AUG_ISFAIL(result = daemonise_())
        || AUG_ISFAIL(result = lockfile_(pidfile))
        || AUG_ISFAIL(result = closein_())
        || AUG_ISFAIL(result = aug_initserv()))
        return result;

    result = aug_runserv();
    aug_termserv();
    return result;
}
