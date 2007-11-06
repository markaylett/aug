/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errinfo.h"
#include "augsys/errno.h"

#include <signal.h>
#include <stdlib.h>  /* NULL */
#include <strings.h> /* bzero() */

#if ENABLE_THREADS
# include <pthread.h>
#endif /* ENABLE_THREADS */

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

static void
sethandler_(struct sigaction* sa, void (*handler)(int))
{
#if !defined(__CYGWIN__)
    sa->sa_handler = handler;
#else /* __CYGWIN__ */
    /* sa_handler is not defined when using -std=c99.  Cygwin bug? */
    *(void (**)(int))sa = handler;
#endif
}

AUGSRV_API int
aug_signalhandler(void (*handler)(int))
{
    int i;
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    for (i = 0; i < sizeof(handlers_) / sizeof(handlers_[0]); ++i) {
        sethandler_(&sa, handlers_[i].dfl_ ? SIG_DFL : handler);
        if (-1 == sigaction(handlers_[i].sig_, &sa, NULL)) {
            aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
            return -1;
        }
    }
    return 0;
}

AUGSRV_API int
aug_blocksignals(void)
{
    sigset_t set;
    sigfillset(&set);

    /* Do not block these. */

    sigdelset(&set, SIGABRT);
    sigdelset(&set, SIGFPE);
    sigdelset(&set, SIGILL);
    sigdelset(&set, SIGSEGV);
    sigdelset(&set, SIGBUS);

#if ENABLE_THREADS
    if (0 != (errno = pthread_sigmask(SIG_SETMASK, &set, NULL))) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
#else /* !ENABLE_THREADS */
    if (-1 == sigprocmask(SIG_SETMASK, &set, NULL)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
#endif /* !ENABLE_THREADS */
    return 0;
}

AUGSRV_API int
aug_unblocksignals(void)
{
    sigset_t set;
    sigemptyset(&set);
#if ENABLE_THREADS
    if (0 != (errno = pthread_sigmask(SIG_SETMASK, &set, NULL))) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
#else /* !ENABLE_THREADS */
    if (-1 == sigprocmask(SIG_SETMASK, &set, NULL)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
#endif /* !ENABLE_THREADS */
    return 0;
}
