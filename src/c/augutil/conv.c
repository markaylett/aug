/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
#include "augutil/conv.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <errno.h>
#include <limits.h> /* UINT_MAX */
#include <stdlib.h> /* strtoul() */

AUGUTIL_API unsigned long*
aug_strtoul(unsigned long* dst, const char* src, int base)
{
    char* end;
    unsigned long ul;

    errno = 0;
    ul = strtoul(src, &end, base);

    if (0 != errno) {

        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        dst = NULL;

    } else if ('\0' != *end) {

        /* The string was only partially processed. */

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",
                       AUG_EPARSE, AUG_MSG("partial conversion"));
        dst = NULL;

    } else {

        /* Success. */

        *dst = ul;
    }
    return dst;
}

AUGUTIL_API unsigned*
aug_strtoui(unsigned* dst, const char* src, int base)
{
    unsigned long ul;
    if (!aug_strtoul(&ul, src, base))
        return NULL;

    if (UINT_MAX < ul) {

        /* Bounds exceeded for target type. */

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",
                       AUG_ELIMIT, AUG_MSG("max integer value exceeded"));
        return NULL;
    }

    *dst = (unsigned)ul;
    return dst;
}

AUGUTIL_API unsigned short*
aug_strtous(unsigned short* dst, const char* src, int base)
{
    unsigned long ul;
    if (!aug_strtoul(&ul, src, base))
        return NULL;

    if (USHRT_MAX < ul) {

        /* Bounds exceeded for target type. */

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",
                       AUG_ELIMIT, AUG_MSG("max integer value exceeded"));
        return NULL;
    }

    *dst = (unsigned short)ul;
    return dst;
}
