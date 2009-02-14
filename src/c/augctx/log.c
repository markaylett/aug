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
#define AUGCTX_BUILD
#include "augctx/log.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include <errno.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
# define vsnprintf _vsnprintf
#endif /* _WIN32 */

static void*
cast_(aug_log* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_logid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
retain_(aug_log* obj)
{
}

static void
release_(aug_log* obj)
{
}

static aug_result
vwritelog_(aug_log* obj, int level, const char* format, va_list args)
{
    char buf[AUG_MAXLINE];
    FILE* file = level > AUG_LOGWARN ? stdout : stderr;
    int ret;

    /* Null termination is _not_ guaranteed by snprintf(). */

    ret = vsnprintf(buf, sizeof(buf), format, args);
    AUG_SNTRUNCF(buf, sizeof(buf), ret);

    if (ret < 0)
        return AUG_FAILERROR;

    fprintf(file, "%s\n", buf);
    fflush(file);
    return AUG_SUCCESS;
}

static const struct aug_logvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    vwritelog_
};

static aug_log stdlog_ = { &vtbl_, NULL };

AUGCTX_API aug_log*
aug_getstdlog(void)
{
    return &stdlog_;
}
