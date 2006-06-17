/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/string.h"

#if HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */

#if !defined(_WIN32)
# include <strings.h> /* strncasecmp() */
# include "augsys/posix/string.c"
#else /* _WIN32 */
# include "augsys/win32/string.c"
# define strcasecmp  _stricmp
# define strncasecmp _strnicmp
#endif /* _WIN32 */

#include "augsys/log.h"

#include <errno.h>

AUGSYS_API int
aug_perror(const char* s)
{
    return aug_error("%s: %s", s, aug_strerror(errno));
}

AUGSYS_API size_t
aug_strlcpy(char* dst, const char* src, size_t size)
{
#if HAVE_STRLCPY

    return strlcpy(dst, src, size);

#else /* !HAVE_STRLCPY */

    char ch;
    size_t len;

    for (len = 0; '\0' != (ch = src[len]); ++len)
        if (len < size)
            dst[len] = ch;

    if (len < size)
        dst[len] = '\0';
    else if (0 < size)
        dst[size - 1] = '\0';

    return len;

#endif /* !HAVE_STRLCPY */
}

AUGSYS_API int
aug_strcasecmp(const char* lhs, const char* rhs)
{
    return strcasecmp(lhs, rhs);
}

AUGSYS_API int
aug_strncasecmp(const char* lhs, const char* rhs, size_t size)
{
    return strncasecmp(lhs, rhs, size);
}
