/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/pwd.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augutil/posix/pwd.c"
#else /* _WIN32 */
# include "augutil/win32/pwd.c"
#endif /* _WIN32 */

#include <string.h>

AUGUTIL_API char*
aug_digestpass(const char* username, const char* realm,
               const char* password, aug_md5base64_t base64)
{
    struct aug_md5context md5ctx;
    unsigned char ha1[16];

    aug_initmd5(&md5ctx);
    aug_appendmd5(&md5ctx, (unsigned char*)username,
                  (unsigned)strlen(username));
    aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
    aug_appendmd5(&md5ctx, (unsigned char*)realm,
                  (unsigned)strlen(realm));
    aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
    aug_appendmd5(&md5ctx, (unsigned char*)password,
                  (unsigned)strlen(password));
    aug_finishmd5(ha1, &md5ctx);
    aug_md5base64(ha1, base64);
    return base64;
}
