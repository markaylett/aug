/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/log.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/errinfo.h"
#include "augsys/lock.h"
#include "augsys/log.h"
#include "augsys/time.h"
#include "augsys/unistd.h"  /* write() */
#include "augsys/utility.h" /* aug_threadid() */

#include <errno.h>          /* EINTR */
#include <stdio.h>

#if defined(_WIN32)
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

static const char LABELS_[][7] = {
    "CRIT  ",
    "ERROR ",
    "WARN  ",
    "NOTICE",
    "INFO  ",
    "DEBUG "
};

static int
localtime_(struct tm* res)
{
    struct timeval tv;
    if (-1 == aug_gettimeofday(&tv, NULL))
        return -1;

    if (!aug_localtime(&tv.tv_sec, res))
        return -1;

    return tv.tv_usec / 1000;
}

static int
writeall_(int fd, const char* buf, size_t n)
{
    /* Ensure all bytes are written and ignore any interrupts. */

    while (0 != n) {

#if !defined(_WIN32)
        int ret = write(fd, buf, n);
#else /* _WIN32 */
        int ret = write(fd, buf, (unsigned)n);
#endif /* _WIN32 */
        if (-1 == ret) {
            if (EINTR == errno)
                continue;

            aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
            return -1;
        }
        buf += ret, n -= ret;
    }
    return 0;
}

AUGUTIL_API const char*
aug_loglabel(int loglevel)
{
    if (sizeof(LABELS_) / sizeof(LABELS_[0]) <= (size_t)loglevel)
        loglevel = AUG_LOGDEBUG0;

    return LABELS_[loglevel];
}

AUGUTIL_API int
aug_vformatlog(char* buf, size_t* n, int loglevel, const char* format,
               va_list args)
{
    int ms, ret;
    size_t size = *n;
    struct tm tm;

    /* At least one character is needed for the null-terminator. */

    if (0 == size) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EBOUND,
                       AUG_MSG("size cannot be zero"));
        return -1;
    }

    if (-1 == (ms = localtime_(&tm)))
        return -1;

    /* The return value from the strftime function is either a) the number of
       characters copied to the buffer, excluding the null terminator, or b)
       zero, indicating an error. */

    if (0 == (ret = (int)strftime(buf, size, AUG_TIMEFORMAT, &tm)))
        goto done;

    buf += ret, size -= ret;


    /* The return value from the snprintf function is either a) the number of
       characters required, excluding the null terminator, or b) a negative
       value, indicating an error. */

#if ENABLE_THREADS
    if (0 > (ret = snprintf(buf, size, ".%03d %08x %s ", ms, aug_threadid(),
                            aug_loglabel(loglevel)))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EFORMAT,
                       AUG_MSG("broken format specification"));
        return -1;
    }
#else /* !ENABLE_THREADS */
    if (0 > (ret = snprintf(buf, size, ".%03d %s ", ms,
                            aug_loglabel(loglevel)))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EFORMAT,
                       AUG_MSG("broken format specification"));
        return -1;
    }
#endif /* !ENABLE_THREADS */

    /* Adjust the return value to be the actual number of characters copied,
       where truncation has occured. */

    if ((size_t)ret >= size) {
        ret = (int)size - 1;
        goto done;
    }

    buf += ret, size -= ret;

    if (0 > (ret = vsnprintf(buf, size, format, args))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EFORMAT,
                       AUG_MSG("broken format specification '%s'"), format);
        return -1;
    }

    /* Adjust the return value to be the actual number of characters copied,
       where truncation has occured. */

    if ((size_t)ret >= size)
        ret = (int)size - 1;

 done:

    buf += ret, size -= ret;
    *buf = '\0';

    /* Set output parameter to be total number of characters copied. */

    *n -= size;
    return 0;
}

AUGUTIL_API int
aug_formatlog(char* buf, size_t* n, int loglevel, const char* format, ...)
{
    int ret;
    va_list args;

    va_start(args, format);
    ret = aug_vformatlog(buf, n, loglevel, format, args);
    va_end(args);

    return ret;
}

AUGUTIL_API int
aug_daemonlogger(int loglevel, const char* format, va_list args)
{
    char buf[AUG_MAXLINE];
    size_t n = sizeof(buf);

    if (-1 == aug_vformatlog(buf, &n, loglevel, format, args))
        return -1;

#if defined(_WIN32) && !defined(NDEBUG)
    aug_lock();
    OutputDebugString(buf);
    OutputDebugString("\n");
    aug_unlock();
#endif /* _WIN32 && !NDEBUG */

    buf[n] = '\n';
    return writeall_(loglevel > AUG_LOGWARN ? STDOUT_FILENO : STDERR_FILENO,
                     buf, n + 1);
}
