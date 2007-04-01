/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/auth.h"
#include "augsys/defs.h"

AUG_RCSID("$Id:$");

#include "augutil/md5.h"

#include <string.h>

/* Derived from example in rfc2617 - HTTP Authentication: Basic and Digest
   Access Authentication.
*/

/*
   The following example assumes that an access-protected document is
   being requested from the server via a GET request. The URI of the
   document is "http://www.nowhere.org/dir/index.html". Both client and
   server know that the username for this document is "Mufasa", and the
   password is "Circle Of Life" (with one space between each of the
   three words).

   The first time the client requests the document, no Authorization
   header is sent, so the server responds with:

         HTTP/1.1 401 Unauthorized
         WWW-Authenticate: Digest
                 realm="testrealm@host.com",
                 qop="auth,auth-int",
                 nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093",
                 opaque="5ccc069c403ebaf9f0171e9517f40e41"

   The client may prompt the user for the username and password, after
   which it will respond with a new request, including the following
   Authorization header:

         Authorization: Digest username="Mufasa",
                 realm="testrealm@host.com",
                 nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093",
                 uri="/dir/index.html",
                 qop=auth,
                 nc=00000001,
                 cnonce="0a4f113b",
                 response="6629fae49393a05397450978507c4ef1",
                 opaque="5ccc069c403ebaf9f0171e9517f40e41"
*/

static void
cvthex_(const unsigned char bin[16], AUG_HASHHEX hex)
{
    unsigned short i;
    unsigned char j;

    for (i = 0; i < 16; i++) {
        j = (bin[i] >> 4) & 0xf;
        if (j <= 9)
            hex[i*2] = (j + '0');
        else
            hex[i*2] = (j + 'a' - 10);
        j = bin[i] & 0xf;
        if (j <= 9)
            hex[i*2+1] = (j + '0');
        else
            hex[i*2+1] = (j + 'a' - 10);
    }
    hex[AUG_HASHHEXLEN] = '\0';
}

/* calculate H(A1) as per spec */

AUGNET_API void
aug_digestha1(const char* alg, const char* username,
              const char* realm, const char* password,
              const char* nonce, const char* cnonce, AUG_HASHHEX sessionkey)
{
    struct aug_md5context md5ctx;
    unsigned char ha1[16];

    aug_initmd5(&md5ctx);
    aug_appendmd5(&md5ctx, (unsigned char*)username,
                  (unsigned int)strlen(username));
    aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
    aug_appendmd5(&md5ctx, (unsigned char*)realm,
                  (unsigned int)strlen(realm));
    aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
    aug_appendmd5(&md5ctx, (unsigned char*)password,
                  (unsigned int)strlen(password));
    aug_finishmd5(ha1, &md5ctx);

    if (stricmp(alg, "md5-sess") == 0) {
        aug_initmd5(&md5ctx);
        aug_appendmd5(&md5ctx, (unsigned char*)ha1, 16);
        aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
        aug_appendmd5(&md5ctx, (unsigned char*)nonce,
                      (unsigned int)strlen(nonce));
        aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
        aug_appendmd5(&md5ctx, (unsigned char*)cnonce,
                      (unsigned int)strlen(cnonce));
        aug_finishmd5(ha1, &md5ctx);
    }

    cvthex_(ha1, sessionkey);
}

/* calculate request-digest/response-digest as per HTTP Digest spec
   H(A1)
   nonce from server
   8 hex digits
   client nonce
   qop-value: "", "auth", "auth-int"
   method from the request
   requested URL
   H(entity body) if qop="auth-int"
   request-digest or response-digest
*/

AUGNET_API void
aug_digestresponse(const AUG_HASHHEX ha1, const char* nonce,
                   const char* noncecount, const char* cnonce,
                   const char* qop, const char* method,
                   const char* digesturi, const AUG_HASHHEX hentity,
                   AUG_HASHHEX response)
{
    struct aug_md5context md5ctx;
    unsigned char ha2[16];
    unsigned char resphash[16];
    AUG_HASHHEX ha2hex;

    /* calculate H(A2) */

    aug_initmd5(&md5ctx);
    aug_appendmd5(&md5ctx, (unsigned char*)method,
                  (unsigned int)strlen(method));
    aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
    aug_appendmd5(&md5ctx, (unsigned char*)digesturi,
                  (unsigned int)strlen(digesturi));

    if (stricmp(qop, "auth-int") == 0) {
        aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
        aug_appendmd5(&md5ctx, (unsigned char*)hentity, AUG_HASHHEXLEN);
    }

    aug_finishmd5(ha2, &md5ctx);
    cvthex_(ha2, ha2hex);

    /* calculate response */

    aug_initmd5(&md5ctx);
    aug_appendmd5(&md5ctx, (unsigned char*)ha1, AUG_HASHHEXLEN);
    aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
    aug_appendmd5(&md5ctx, (unsigned char*)nonce,
                  (unsigned int)strlen(nonce));
    aug_appendmd5(&md5ctx, (unsigned char*)":", 1);

    if (*qop) {
        aug_appendmd5(&md5ctx, (unsigned char*)noncecount,
                      (unsigned int)strlen(noncecount));
        aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
        aug_appendmd5(&md5ctx, (unsigned char*)cnonce,
                      (unsigned int)strlen(cnonce));
        aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
        aug_appendmd5(&md5ctx, (unsigned char*)qop,
                      (unsigned int)strlen(qop));
        aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
    }

    aug_appendmd5(&md5ctx, (unsigned char*)ha2hex, AUG_HASHHEXLEN);
    aug_finishmd5(resphash, &md5ctx);
    cvthex_(resphash, response);
}
