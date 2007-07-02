/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#undef __STRICT_ANSI__ /* _stricmp() */
#define AUGSYS_BUILD
#include "augsys/string.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include <strings.h> /* strncasecmp() */
# include "augsys/posix/string.c"
#else /* _WIN32 */
# include "augsys/win32/string.c"
# define strcasecmp  _stricmp
# define strncasecmp _strnicmp
#endif /* _WIN32 */

#include "augsys/log.h"

#include <ctype.h>
#include <errno.h>

AUGSYS_API void*
aug_memfrob(void* dst, size_t size)
{
    char* ptr = (char*)dst;
    while (size)
        ptr[--size] ^= 42;
    return dst;
}

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

    /* Thanks to Dan Cross for this public domain implementation of
       strlcpy(). */

    size_t len, srclen;
    srclen = strlen(src);
    if (--size <= 0) return(srclen);
    len = (size < srclen) ? size : srclen;
    memmove(dst, src, len);
    dst[len] = '\0';
    return(srclen);

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

AUGSYS_API const char*
aug_strcasestr(const char* haystack, const char* needle)
{
    /* Thanks to Fred Cole and Bob Stout for this public domain implementation
       of strcasestr(). */

    char *pptr, *sptr, *start;
    unsigned slen, plen;

    for (start = (char *)haystack,
             pptr  = (char *)needle,
             slen  = (unsigned)strlen(haystack),
             plen  = (unsigned)strlen(needle);

         /* while string length not shorter than pattern length */

         slen >= plen;
         start++, slen--) {

        /* find start of pattern in string */
        while (toupper(*start) != toupper(*needle)) {
            start++;
            slen--;

            /* if pattern longer than string */

            if (slen < plen)
                return(NULL);
        }

        sptr = start;
        pptr = (char *)needle;

        while (toupper(*sptr) == toupper(*pptr)) {
            sptr++;
            pptr++;

            /* if end of pattern then pattern was found */

            if ('\0' == *pptr)
                return (start);
        }
    }
    return(NULL);
}
