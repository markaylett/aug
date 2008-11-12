/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"

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
#endif /* __CYGWIN__ */
}

AUGSRV_API aug_result
aug_setsighandler(void (*handler)(int))
{
    int i;
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    for (i = 0; i < sizeof(handlers_) / sizeof(handlers_[0]); ++i) {
        sethandler_(&sa, handlers_[i].dfl_ ? SIG_DFL : handler);
        if (-1 == sigaction(handlers_[i].sig_, &sa, NULL))
            return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
    }
    return AUG_SUCCESS;
}

AUGSRV_API aug_result
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
    if (0 != (errno = pthread_sigmask(SIG_SETMASK, &set, NULL)))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

#else /* !ENABLE_THREADS */
    if (-1 == sigprocmask(SIG_SETMASK, &set, NULL))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
#endif /* !ENABLE_THREADS */

    return AUG_SUCCESS;
}

AUGSRV_API aug_result
aug_unblocksignals(void)
{
    sigset_t set;
    sigemptyset(&set);
#if ENABLE_THREADS
    if (0 != (errno = pthread_sigmask(SIG_SETMASK, &set, NULL)))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
#else /* !ENABLE_THREADS */
    if (-1 == sigprocmask(SIG_SETMASK, &set, NULL))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
#endif /* !ENABLE_THREADS */
    return AUG_SUCCESS;
}
