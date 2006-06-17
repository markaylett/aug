/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include <signal.h>
#include <stdlib.h>  /* NULL */
#include <strings.h> /* bzero() */

#if defined(_MT)
# include <pthread.h>
#endif /* _MT */

static const struct {
    int sig_;
    int dfl_;
} handlers_[] = {
    { SIGHUP, 0 },
    { SIGINT, 0 },
    { SIGUSR1, 0 },
    { SIGPIPE, 1 },
    { SIGTERM, 0 }
};

AUGSRV_API int
aug_signalhandler(void (*handler)(int))
{
    int i;
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    for (i = 0; i < sizeof(handlers_) / sizeof(handlers_[0]); ++i) {
        sa.sa_handler = handlers_[i].dfl_ ? SIG_DFL : handler;
        if (-1 == sigaction(handlers_[i].sig_, &sa, NULL))
            return -1;
    }
    return 0;
}

AUGSRV_API int
aug_blocksignals(void)
{
    sigset_t set;
    sigfillset(&set);
#if !defined(_MT)
    return sigprocmask(SIG_SETMASK, &set, NULL);
#else /* _MT */
    return pthread_sigmask(SIG_SETMASK, &set, NULL);
#endif /* _MT */
}

AUGSRV_API int
aug_unblocksignals(void)
{
    sigset_t set;
    sigemptyset(&set);
#if !defined(_MT)
    return sigprocmask(SIG_SETMASK, &set, NULL);
#else /* _MT */
    return pthread_sigmask(SIG_SETMASK, &set, NULL);
#endif /* _MT */
}
