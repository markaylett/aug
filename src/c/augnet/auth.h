/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_AUTH_H
#define AUGNET_AUTH_H

#include "augnet/config.h"

#include "augutil/md5.h"

AUGNET_API char*
aug_digestha1(const char* alg, const char* username, const char* realm,
              const char* password, const char* nonce, const char* cnonce,
              aug_md5base64_t base64);

AUGNET_API char*
aug_digestresponse(const aug_md5base64_t ha1, const char* nonce,
                   const char* nc, const char* cnonce, const char* qop,
                   const char* method, const char* uri,
                   const aug_md5base64_t hentity, aug_md5base64_t base64);

#endif /* AUGNET_AUTH_H */
