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
    aug_bool dfl_;
} handlers_[] = {
    { SIGHUP,  AUG_FALSE },
    { SIGINT,  AUG_FALSE },
    { SIGUSR1, AUG_FALSE },
    { SIGPIPE, AUG_TRUE  },
    { SIGTERM, AUG_FALSE }
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

AUGSERV_API aug_result
aug_setsighandler_I(void (*handler)(int))
{
    int i;
    struct sigaction sa;
    if (!handler)
        handler = SIG_DFL;

    bzero(&sa, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    for (i = 0; i < sizeof(handlers_) / sizeof(handlers_[0]); ++i) {
        sethandler_(&sa, handlers_[i].dfl_ ? SIG_DFL : handler);
        /* EXCEPT: aug_setsighandler_I -> sigaction; */
        /* EXCEPT: sigaction -> EINTR; */
        if (sigaction(handlers_[i].sig_, &sa, NULL) < 0) {
            aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
            return -1;
        }
    }
    return 0;
}

AUGSERV_API aug_result
aug_sigblock(void)
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
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

#else /* !ENABLE_THREADS */
    /* SIGCALL: sigprocmask: */
    if (sigprocmask(SIG_SETMASK, &set, NULL) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }
#endif /* !ENABLE_THREADS */

    return 0;
}

AUGSERV_API aug_result
aug_sigunblock(void)
{
    sigset_t set;
    sigemptyset(&set);
#if ENABLE_THREADS
    if (0 != (errno = pthread_sigmask(SIG_SETMASK, &set, NULL))) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }
#else /* !ENABLE_THREADS */
    /* SIGCALL: sigprocmask: */
    if (sigprocmask(SIG_SETMASK, &set, NULL) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }
#endif /* !ENABLE_THREADS */
    return 0;
}
