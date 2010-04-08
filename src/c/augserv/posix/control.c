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
#include "augserv/base.h"
#include "augserv/options.h"
#include "augserv/types.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/string.h"

#include "augext/log.h"

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>          /* struct flock */
#include <signal.h>         /* kill() */
#include <strings.h>        /* bzero() */
#include <unistd.h>

static aug_result
flock_BI_(struct flock* fl, int fd, int cmd, int type)
{
    bzero(fl, sizeof(*fl));

    fl->l_type = type;
    fl->l_whence = SEEK_SET;
    fl->l_start = 0;
    fl->l_len = 0;

    /* EXCEPT: flock_BI_ -> fcntl; */
    /* EXCEPT: fcntl -> EAGAIN; */
    /* EXCEPT: fcntl -> EINTR; */
    if (fcntl(fd, cmd, fl) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static aug_result
send_BI_(int fd, pid_t pid, int event)
{
    struct flock fl;

    switch (event) {
    case AUG_EVENTRECONF:
        if (kill(pid, SIGHUP) < 0) {
            aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
            return -1;
        }
        break;
    case AUG_EVENTSTATUS:
        if (kill(pid, SIGUSR1) < 0) {
            aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
            return -1;
        }
        break;
    case AUG_EVENTSTOP:
        if (kill(pid, SIGTERM) < 0) {
            aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
            return -1;
        }

        /* Wait for daemon process to release lock. */

        /* EXCEPT: send_BI_ -> flock_BI_; */

        if (flock_BI_(&fl, fd, F_SETLKW, F_RDLCK) < 0)
            return -1;

        /* The lock has been obtained; daemon process must have stopped. */

        break;
    default:

        /* Invalid command. */

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid control command [%d]"), (int)event);
        return -1;
    }
    return 0;
}

AUGSERV_API aug_result
aug_start_N(const struct aug_options* options)
{
    aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
                    AUG_MSG("aug_start_N() not supported"));
    return -1;
}

AUGSERV_API aug_result
aug_control_BIN(const struct aug_options* options, int event)
{
    const char* pidfile;
    struct flock fl;
    int fd;
    aug_result result;

    if (aug_readservconf(AUG_CONFFILE(options), options->batch_,
                         AUG_TRUE) < 0)
        return -1;

    if (!(pidfile = aug_getservopt(AUG_OPTPIDFILE))) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("option 'AUG_OPTPIDFILE' not set"));
        return -1;
    }

    /* Check for existence of file. */

    if (access(pidfile, F_OK) < 0) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EEXIST,
                        AUG_MSG("pidfile does not exist: %s"), pidfile);
        return -1;
    }

    /* EXCEPT: aug_control_BIN -> open; */
    /* EXCEPT: open -> ENOENT; */
	if ((fd = open(pidfile, O_RDONLY)) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    /* Attempt to obtain shared lock. */

    /* EXCEPT: aug_control_BIN -> flock_BI_; */

    if (flock_BI_(&fl, fd, F_SETLK, F_RDLCK) < 0) {

        /* As expected, the daemon process has an exclusive lock on the pid
           file.  Use F_GETLK to obtain the pid of the daemon process. */

        /* EXCEPT: aug_control_BIN -> flock_BI_; */

        if (0 <= flock_BI_(&fl, fd, F_GETLK, F_RDLCK)) {

            /* Although a lock-manager allows locking over NFS, the returned
               pid is probably zero because it could be the pid of a process
               on another node. */

            if (0 == fl.l_pid) {

                aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EIO,
                                AUG_MSG("lockfile on NFS mount: %s"),
                                pidfile);
                result = -1;

            } else {
                /* EXCEPT: aug_control_BIN -> send_BI_; */
                result = send_BI_(fd, fl.l_pid, event);
            }
        } else
            result -1;

    } else {

        /* The lock was obtained, therefore, the daemon process cannot be
           running. */

        /* EXCEPT: aug_control_BIN -> unlink; */
        /* EXCEPT: unlink -> ENOENT; */
        if (unlink(pidfile) < 0)
            aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        else
            aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EEXIST,
                            AUG_MSG("server process is not running"));
        result = -1;
    }

    close(fd);
    return result;
}

AUGSERV_API aug_result
aug_install(const struct aug_options* options)
{
    return 0;
}

AUGSERV_API aug_result
aug_uninstall(const struct aug_options* options)
{
    return 0;
}
