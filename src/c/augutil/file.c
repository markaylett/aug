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
#include "augutil/file.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"

#include "augtypes.h" /* AUG_EPARSE */

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define SPACE_ "\t\v\f "

static size_t
rtrim_(const char* s, size_t n)
{
    while (0 < n && isspace((int)s[n - 1]))
        --n;
    return n;
}

AUGUTIL_API aug_result
aug_readconf(const char* path, aug_confcb_t cb, void* arg)
{
    char buf[AUG_MAXLINE];
    char* name;
    const char* value;
    aug_result result = AUG_SUCCESS;

    FILE* fp = fopen(path, "r");
    if (!fp)
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    while (fgets(buf, sizeof(buf), fp)) {

        /* Trim trailing whitespace from the line. */

        buf[rtrim_(buf, strlen(buf))] = '\0';

        /* Trim leading whitespace from the line. */

        name = buf + strspn(buf, SPACE_);

        /* Ignore if either a blank line or comment line.  Note: because there
           is no notion of an escape character, for simplicity, comments
           cannot be placed after name/value pairs. */

        if ('\0' == *name || '#' == *name)
            continue;

        /* Find the token separating the name and value - this is required. */

        if (!(value = strchr(name, '='))) {

            aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EPARSE,
                           AUG_MSG("missing token separator"));
            result = AUG_FAILERROR;
            break;
        }

        /* Trim trailing whitespace from the name - the name cannot be
           blank. */

        name[rtrim_(name, value - name)] = '\0';
        if ('\0' == *name) {
            aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EPARSE,
                           AUG_MSG("missing name part"));
            result = AUG_FAILERROR;
            break;
        }

        /* Skip past the separator. */

        ++value;

        /* Trim leading whitespace from the value. */

        value += strspn(value, SPACE_);

        if (AUG_ISFAIL(result = (*cb)(arg, name, value)))
            break;
    }

    fclose(fp);
    return result;
}
