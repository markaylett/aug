/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augctx/string.h"

#include <pwd.h>
#include <unistd.h>

/* Required by solaris build. */

char* getpass(const char*);

AUGUTIL_API char*
aug_getpass(const char* prompt, char* buf, size_t len)
{
    char* pass = getpass(prompt);
    aug_strlcpy(buf, pass, len);
    if ((len = strlen(pass)))
        memset(pass, 0, len);
    return buf;
}
