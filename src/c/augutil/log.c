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
#define AUGUTIL_BUILD
#include "augutil/log.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/time.h"
#include "augsys/utility.h" /* aug_threadid() */

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/lock.h"

#include <assert.h>
#include <errno.h>          /* EINTR */
#include <stdio.h>
#include <string.h>         /* strcmp() */

#if !defined(_WIN32)
# include <unistd.h>
#else /* _WIN32 */
# include <io.h>
# define snprintf _snprintf
# define vsnprintf _vsnprintf
#endif /* _WIN32 */

#if !defined(STDOUT_FILENO)
# define STDOUT_FILENO 1
#endif /* !STDOUT_FILENO */

#if !defined(STDERR_FILENO)
# define STDERR_FILENO 2
#endif /* !STDERR_FILENO */

/* The time format can defined by the build process if required. */

#if !defined(AUG_TIMEFORMAT)
# define AUG_TIMEFORMAT "%b %d %H:%M:%S"
#endif /* !AUG_TIMEFORMAT */

static const char* LABELS_[] = {
    "CRIT",
    "ERROR",
    "WARN",
    "NOTICE",
    "INFO",
    "DEBUG"
};

static aug_rsize
localtime_(aug_clock* clock, struct tm* res)
{
    struct aug_timeval tv;
    aug_verify(aug_gettimeofday(clock, &tv));

    if (!aug_localtime(&tv.tv_sec, res))
        return AUG_FAILERROR;

    return AUG_MKRESULT(tv.tv_usec / 1000);
}

static aug_result
writeall_(int fd, const char* buf, size_t n)
{
    /* Ensure all bytes are written and ignore any interrupts. */

    while (0 != n) {

#if !defined(_WIN32)
        int ret = write(fd, buf, n);
#else /* _WIN32 */
        int ret = _write(fd, buf, (unsigned)n);
#endif /* _WIN32 */
        if (-1 == ret) {
            if (EINTR == errno)
                continue;

            return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        }
        buf += ret, n -= ret;
    }
    return AUG_SUCCESS;
}

struct impl_ {
    aug_log log_;
    int refs_;
    aug_mpool* mpool_;
    aug_clock* clock_;
};

static void*
cast_(aug_log* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_logid)) {
        aug_retain(ob);
        return ob;
    }
    return NULL;
}

static void
retain_(aug_log* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, log_, ob);
    ++impl->refs_;
}

static void
release_(aug_log* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, log_, ob);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        aug_clock* clock = impl->clock_;
        aug_freemem(mpool, impl);
        aug_release(clock);
        aug_release(mpool);
    }
}

static aug_result
vwritelog_(aug_log* ob, int level, const char* format, va_list args)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, log_, ob);

    char buf[AUG_MAXLINE];
    size_t n = sizeof(buf);

    aug_verify(aug_vformatlog(buf, &n, impl->clock_, level, format, args));

#if defined(_WIN32) && !defined(NDEBUG)
    aug_lock();
    OutputDebugString(buf);
    OutputDebugString("\n");
    aug_unlock();
#endif /* _WIN32 && !NDEBUG */

    buf[n] = '\n';
    return writeall_(level > AUG_LOGWARN ? STDOUT_FILENO : STDERR_FILENO,
                     buf, n + 1);
}

static const struct aug_logvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    vwritelog_
};

AUGUTIL_API const char*
aug_loglabel(int level)
{
    if (sizeof(LABELS_) / sizeof(LABELS_[0]) <= (size_t)level)
        level = AUG_LOGDEBUG0;

    return LABELS_[level];
}

AUGUTIL_API aug_result
aug_vformatlog(char* buf, size_t* n, aug_clock* clock, int level,
               const char* format, va_list args)
{
    size_t size;
    struct tm tm;
    aug_rsize ms;
    int ret;

    /* At least one character is needed for the null-terminator. */

    assert(buf && n && *n && format);
    size = *n;

    if (aug_isfail(ms = localtime_(clock, &tm)))
        return ms;

    /* The return value from the strftime() function is either a) the number
       of characters copied to the buffer, excluding the null terminator, or
       b) zero, indicating an error. */

    if (0 == (ret = (int)strftime(buf, size, AUG_TIMEFORMAT, &tm)))
        goto done;

    buf += ret, size -= ret;

    /* Null termination is _not_ guaranteed by snprintf(). */

#if ENABLE_THREADS
    ret = snprintf(buf, size, ".%03d %08x %-6s ", (int)AUG_RESULT(ms),
                   aug_threadid(), aug_loglabel(level));
#else /* !ENABLE_THREADS */
    ret = snprintf(buf, size, ".%03d %-6s ", (int)AUG_RESULT(ms),
                   aug_loglabel(level));
#endif /* !ENABLE_THREADS */

    AUG_SNTRUNCF(buf, size, ret);

    if (ret < 0)
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    buf += ret, size -= ret;

    ret = vsnprintf(buf, size, format, args);
    AUG_SNTRUNCF(buf, size, ret);

    if (ret < 0)
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

 done:

    buf += ret, size -= ret;
    *buf = '\0';

    /* Set output parameter to be total number of characters copied. */

    *n -= size;
    return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_formatlog(char* buf, size_t* n, aug_clock* clock, int level,
              const char* format, ...)
{
    aug_result result;
    va_list args;

    va_start(args, format);
    result = aug_vformatlog(buf, n, clock, level, format, args);
    va_end(args);

    return result;
}

AUGUTIL_API aug_log*
aug_createdaemonlog(aug_mpool* mpool, aug_clock* clock)
{
    struct impl_* impl = aug_allocmem(mpool, sizeof(struct impl_));
    if (!impl)
        return NULL;

    impl->log_.vtbl_ = &vtbl_;
    impl->log_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->clock_ = clock;

    aug_retain(mpool);
    aug_retain(clock);
    return &impl->log_;
}

AUGUTIL_API aug_result
aug_setdaemonlog(aug_ctx* ctx)
{
    aug_mpool* mpool = aug_getmpool(ctx);
    aug_clock* clock = aug_getclock(ctx);
    aug_log* log = aug_createdaemonlog(mpool, clock);
    aug_release(clock);
    aug_release(mpool);
    if (!log)
        return AUG_FAILERROR;
    aug_setlog(ctx, log);
    return AUG_SUCCESS;
}
