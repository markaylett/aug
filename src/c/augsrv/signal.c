/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRV_BUILD
#include "augsrv/signal.h"

#if !defined(_WIN32)
# include "augsrv/posix/signal.c"
#else /* _WIN32 */
# include "augsrv/win32/signal.c"
# define SIGHUP  1
# define SIGUSR1 10
# define SIGALRM 14
# define SIGCHLD 17
#endif /* _WIN32 */

#include "augsrv/global.h"

#include "augsys/errno.h"
#include "augsys/unistd.h"

AUGSRV_API enum aug_signal
aug_tosignal(int i)
{
    enum aug_signal sig;
    switch (i) {
    case SIGCHLD:
        sig = AUG_SIGCHILD;
        break;
    case SIGALRM:
        sig = AUG_SIGALARM;
        break;
    case SIGHUP:
        sig = AUG_SIGRECONF;
        break;
    case SIGUSR1:
        sig = AUG_SIGSTATUS;
        break;
    case SIGINT:
    case SIGTERM:
        sig = AUG_SIGSTOP;
        break;
    default:
        sig = AUG_SIGOTHER;
        break;
    }
    return sig;
}

AUGSRV_API int
aug_readsig(enum aug_signal* sig)
{
    char ch;
    int ret = aug_read(aug_sigin(), &ch, sizeof(ch));
    if (sizeof(ch) == ret) {
        *sig = ch;
        return 0;
    }

    /* Set errno for partial read and graceful shutdown. */

    if (-1 != ret)
        errno = EINVAL;

    return -1;
}

AUGSRV_API int
aug_writesig(enum aug_signal sig)
{
    char ch = sig;
    int ret = aug_write(aug_sigout(), &ch, sizeof(ch));
    if (sizeof(ch) == ret)
        return 0;

    /* Set errno for partial write. */

    if (-1 != ret)
        errno = EINVAL;

    return -1;
}
