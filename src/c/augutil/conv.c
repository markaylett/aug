/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/conv.h"

static const char rcsid[] = "$Id:$";

#include "augsys/errinfo.h"

#include <errno.h>
#include <limits.h> /* UINT_MAX */
#include <stdlib.h> /* strtoul() */

AUGUTIL_API int
aug_strtoul(unsigned long* dst, const char* src, int base)
{
    char* end;
    int ret;
    unsigned long ul;

    errno = 0;
    ul = strtoul(src, &end, base);

    if (0 != errno) {

        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        ret = -1;

    } else if ('\0' != *end) {

        /* The string was only partially processed. */

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("partial conversion"));
        ret = -1;

    } else {

        /* Success. */

        *dst = ul;
        ret = 0;
    }
    return ret;
}

AUGUTIL_API int
aug_strtoui(unsigned int* dst, const char* src, int base)
{
    unsigned long ul;
    if (-1 == aug_strtoul(&ul, src, base))
        return -1;

    if (UINT_MAX < ul) {

        /* Bounds exceeded for target type. */

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EBOUND,
                       AUG_MSG("max integer value exceeded"));
        return -1;
    }

    *dst = (unsigned int)ul;
    return 0;
}

AUGUTIL_API int
aug_strtous(unsigned short* dst, const char* src, int base)
{
    unsigned long ul;
    if (-1 == aug_strtoul(&ul, src, base))
        return -1;

    if (USHRT_MAX < ul) {

        /* Bounds exceeded for target type. */

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EBOUND,
                       AUG_MSG("max integer value exceeded"));
        return -1;
    }

    *dst = (unsigned short)ul;
    return 0;
}
