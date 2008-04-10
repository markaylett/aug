/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#undef __STRICT_ANSI__ /* _stricmp() */
#define AUGCTX_BUILD
#include "augctx/string.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include <strings.h> /* strncasecmp() */
#else /* _WIN32 */
# define strcasecmp  _stricmp
# define strncasecmp _strnicmp
#endif /* _WIN32 */

#include <ctype.h>

AUGCTX_API size_t
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

AUGCTX_API int
aug_strcasecmp(const char* lhs, const char* rhs)
{
    return strcasecmp(lhs, rhs);
}

AUGCTX_API int
aug_strncasecmp(const char* lhs, const char* rhs, size_t size)
{
    return strncasecmp(lhs, rhs, size);
}

AUGCTX_API const char*
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
