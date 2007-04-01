/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_AUTH_H
#define AUGNET_AUTH_H

#include "augnet/config.h"

#define AUG_HASHHEXLEN 32

typedef char AUG_HASHHEX[AUG_HASHHEXLEN + 1];

AUGNET_API void
aug_digestha1(const char* alg, const char* username,
              const char* realm, const char* password,
              const char* nonce, const char* cnonce, AUG_HASHHEX sessionkey);

AUGNET_API void
aug_digestresponse(const AUG_HASHHEX ha1, const char* nonce,
                   const char* noncecount, const char* cnonce,
                   const char* qop, const char* method,
                   const char* digesturi, const AUG_HASHHEX hentity,
                   AUG_HASHHEX response);

#endif /* AUGNET_AUTH_H */
