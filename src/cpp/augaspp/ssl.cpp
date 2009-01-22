/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGASPP_BUILD
#include "augaspp/ssl.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if WITH_SSL

# include "augaspp/conn.hpp"

# include <openssl/err.h>
# include <openssl/ssl.h>

using namespace aug;

AUGASPP_API
sslctx::~sslctx() AUG_NOTHROW
{
    SSL_CTX_free(ctx_);
}

AUGASPP_API
sslctx::sslctx()
    : ctx_(SSL_CTX_new(SSLv23_method()))
{
    if (!ctx_)
        throw ssl_error(__FILE__, __LINE__, ERR_get_error());
}

AUGASPP_API void
aug::initssl()
{
    // Global system initialization.

    SSL_library_init();
    SSL_load_error_strings();
}

#endif // WITH_SSL
