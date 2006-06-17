/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/signal.h"

#include "augsys/errno.h"
#include "augsys/unistd.h"

#include <signal.h>

#if defined(_WIN32)
# define SIGHUP  1
# define SIGUSR1 10
#endif /* _WIN32 */

AUGUTIL_API aug_signal_t
aug_tosignal(int i)
{
    aug_signal_t sig;
    switch (i) {
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
        sig = AUG_SIGNONE;
    }
    return sig;
}

AUGUTIL_API int
aug_readsignal(int fd, aug_signal_t* sig)
{
    char ch;
    int ret = aug_read(fd, &ch, sizeof(ch));
    if (sizeof(ch) == ret) {
        *sig = ch;
        return 0;
    }

    /* Set errno for partial read and graceful shutdown. */

    if (-1 != ret)
        errno = EINVAL;

    return -1;
}

AUGUTIL_API int
aug_writesignal(int fd, aug_signal_t sig)
{
    char ch = sig;
    int ret = aug_write(fd, &ch, sizeof(ch));
    if (sizeof(ch) == ret)
        return 0;

    /* Set errno for partial write. */

    if (-1 != ret)
        errno = EINVAL;

    return -1;
}
