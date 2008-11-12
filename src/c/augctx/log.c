/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
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
