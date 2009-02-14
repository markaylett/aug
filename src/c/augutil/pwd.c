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
#include "augutil/pwd.h"
#include "augctx/defs.h"

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
