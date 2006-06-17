/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/log.h"

#include "augsys/defs.h"   /* AUG_MAXLINE */
#include "augsys/time.h"
#include "augsys/unistd.h" /* write() */

#include <errno.h>
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
# if !defined(_WIN32)
#  define AUG_TIMEFORMAT "%h %e %T"
# else /* WIN32 */
#  define AUG_TIMEFORMAT "%c"
# endif /* WIN32 */
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
    time_t clock = time(NULL);
    return -1 == clock || !aug_localtime(&clock, res) ? -1 : 0;
}

static int
writeall_(int fd, const char* buf, size_t n)
{
    /* Ensure all bytes are written and ignore any interrupts. */

    while (0 != n) {

#if !defined(_WIN32)
        int ret = write(fd, buf, n);
#else /* _WIN32 */
        int ret = write(fd, buf, (unsigned int)n);
#endif /* _WIN32 */
        if (-1 == ret) {
            if (EINTR == errno)
                continue;

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
        loglevel = AUG_LOGDEBUG;

    return LABELS_[loglevel];
}

AUGUTIL_API int
aug_vformatlog(char* buf, size_t* n, int loglevel, const char* format,
               va_list args)
{
    int ret;
    size_t size = *n;
    struct tm tm;

    /* At least one character is needed for the null-terminator. */

    if (0 == size) {
        errno = EINVAL;
        return -1;
    }

    if (-1 == localtime_(&tm))
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

    if (0 > (ret = snprintf(buf, size, " %s ", aug_loglabel(loglevel)))) {
        errno = EINVAL;
        return -1;
    }

    /* Adjust the return value to be the actual number of characters copied,
       where truncation has occured. */

    if ((size_t)ret >= size) {
        ret = (int)size - 1;
        goto done;
    }

    buf += ret, size -= ret;

    if (0 > (ret = vsnprintf(buf, size, format, args))) {
        errno = EINVAL;
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

    buf[n] = '\n';

#if 0 && defined(_WIN32)
    OutputDebugString(buf); /* needs nullterm */
#endif /* _WIN32 */
    return writeall_(loglevel > AUG_LOGWARN ? STDOUT_FILENO : STDERR_FILENO,
                     buf, n + 1);
}
