/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_AUTH_H
#define AUGNET_AUTH_H

#include "augnet/config.h"

#include "augutil/md5.h"

AUGNET_API const char*
aug_digestha1(const char* alg, const char* username,
              const char* realm, const char* password,
              const char* nonce, const char* cnonce,
              AUG_MD5BASE64 sessionkey);

AUGNET_API const char*
aug_digestresponse(const AUG_MD5BASE64 ha1, const char* nonce,
                   const char* noncecount, const char* cnonce,
                   const char* qop, const char* method,
                   const char* digesturi, const AUG_MD5BASE64 hentity,
                   AUG_MD5BASE64 response);

#endif /* AUGNET_AUTH_H */
