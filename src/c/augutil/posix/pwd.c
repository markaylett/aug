/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/string.h"

#include <pwd.h>
#include <unistd.h>

AUGUTIL_API char*
aug_getpass(const char* prompt, char* buf, size_t len)
{
	char* pass = getpass(prompt);
	aug_strlcpy(buf, pass, len);
    if ((len = strlen(pass)))
        memset(pass, 0, len);
	return buf;
}
