/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include <signal.h>
#include <stdlib.h>  /* NULL */
#include <strings.h> /* bzero() */

AUGSRV_API int
aug_signalhandler(void (*handler)(int))
{
    const struct {
        int sig_;
        void (*handler_)(int);
    }
    handlers[] = {
        { SIGHUP, handler },
        { SIGINT, handler },
        { SIGUSR1, handler },
        { SIGPIPE, SIG_DFL },
        { SIGTERM, handler }
    };

    int i;
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    for (i = 0; i < sizeof(handlers) / sizeof(handlers[0]); ++i) {
        sa.sa_handler = handlers[i].handler_;
        if (-1 == sigaction(handlers[i].sig_, &sa, NULL))
            return -1;
    }
    return 0;
}

AUGSRV_API int
aug_blocksignals(void)
{
    sigset_t set;
    sigfillset(&set);
    return sigprocmask(SIG_SETMASK, &set, 0);
}

AUGSRV_API int
aug_unblocksignals(void)
{
    sigset_t set;
    sigemptyset(&set);
    return sigprocmask(SIG_SETMASK, &set, 0);
}
