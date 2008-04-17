/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/auth.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/md5.h"

#include "augctx/string.h"

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

/* calculate H(A1) as per spec */

AUGNET_API char*
aug_digestha1(const char* alg, const char* username, const char* realm,
              const char* password, const char* nonce, const char* cnonce,
              aug_md5base64_t base64)
{
    struct aug_md5context md5ctx;
    unsigned char ha1[16];

    aug_initmd5(&md5ctx);
    aug_appendmd5(&md5ctx, (unsigned char*)username,
                  (unsigned)strlen(username));
    aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
    aug_appendmd5(&md5ctx, (unsigned char*)realm, (unsigned)strlen(realm));
    aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
    aug_appendmd5(&md5ctx, (unsigned char*)password,
                  (unsigned)strlen(password));
    aug_finishmd5(ha1, &md5ctx);

    if (aug_strcasecmp(alg, "md5-sess") == 0) {
        aug_initmd5(&md5ctx);
        aug_appendmd5(&md5ctx, (unsigned char*)ha1, 16);
        aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
        aug_appendmd5(&md5ctx, (unsigned char*)nonce,
                      (unsigned)strlen(nonce));
        aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
        aug_appendmd5(&md5ctx, (unsigned char*)cnonce,
                      (unsigned)strlen(cnonce));
        aug_finishmd5(ha1, &md5ctx);
    }

    aug_md5base64(ha1, base64);
    return base64;
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

AUGNET_API char*
aug_digestresponse(const aug_md5base64_t ha1, const char* nonce,
                   const char* nc, const char* cnonce, const char* qop,
                   const char* method, const char* uri,
                   const aug_md5base64_t hentity, aug_md5base64_t base64)
{
    struct aug_md5context md5ctx;
    unsigned char ha2[16];
    unsigned char response[16];
    aug_md5base64_t ha2base64;

    /* calculate H(A2) */

    aug_initmd5(&md5ctx);
    aug_appendmd5(&md5ctx, (unsigned char*)method, (unsigned)strlen(method));
    aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
    aug_appendmd5(&md5ctx, (unsigned char*)uri, (unsigned)strlen(uri));

    if (aug_strcasecmp(qop, "auth-int") == 0) {
        aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
        aug_appendmd5(&md5ctx, (unsigned char*)hentity, AUG_MD5BASE64LEN);
    }

    aug_finishmd5(ha2, &md5ctx);
    aug_md5base64(ha2, ha2base64);

    /* calculate response */

    aug_initmd5(&md5ctx);
    aug_appendmd5(&md5ctx, (unsigned char*)ha1, AUG_MD5BASE64LEN);
    aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
    aug_appendmd5(&md5ctx, (unsigned char*)nonce, (unsigned)strlen(nonce));
    aug_appendmd5(&md5ctx, (unsigned char*)":", 1);

    if (*qop) {
        aug_appendmd5(&md5ctx, (unsigned char*)nc, (unsigned)strlen(nc));
        aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
        aug_appendmd5(&md5ctx, (unsigned char*)cnonce,
                      (unsigned)strlen(cnonce));
        aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
        aug_appendmd5(&md5ctx, (unsigned char*)qop, (unsigned)strlen(qop));
        aug_appendmd5(&md5ctx, (unsigned char*)":", 1);
    }

    aug_appendmd5(&md5ctx, (unsigned char*)ha2base64, AUG_MD5BASE64LEN);

    aug_finishmd5(response, &md5ctx);
    aug_md5base64(response, base64);
    return base64;
}
