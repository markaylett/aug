/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/file.h"

static const char rcsid[] = "$Id:$";

#include "augsys/defs.h"
#include "augsys/errinfo.h"
#include "augsys/errno.h"

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

AUGUTIL_API int
aug_readconf(const char* path, aug_confcb_t cb, const struct aug_var* arg)
{
    char buf[AUG_MAXLINE];
    char* name;
    const char* value;
    int ret = 0;

    FILE* fp = fopen(path, "r");
    if (!fp) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

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

            aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                           AUG_MSG("missing token separator"));
            ret = -1;
            break;
        }

        /* Trim trailing whitespace from the name - the name cannot be
           blank. */

        name[rtrim_(name, value - name)] = '\0';
        if ('\0' == *name) {
            aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                           AUG_MSG("missing name part"));
            ret = -1;
            break;
        }

        /* Skip past the separator. */

        ++value;

        /* Trim leading whitespace from the value. */

        value += strspn(value, SPACE_);

        if (-1 == (*cb)(arg, name, value)) {
            ret = -1;
            break;
        }
    }

    fclose(fp);
    return ret;
}
