/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/ssl.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#if HAVE_OPENSSL_SSL_H

# include "augrtpp/conn.hpp"

# include <openssl/err.h>

using namespace aug;

sslctx::~sslctx() AUG_NOTHROW
{
    SSL_CTX_free(ctx_);
}

sslctx::sslctx()
    : ctx_(SSL_CTX_new(SSLv23_method()))
{
    if (!ctx_)
        throw ssl_error(__FILE__, __LINE__, ERR_get_error());
}

void
aug::initssl()
{
    // Global system initialization.

    SSL_library_init();
    SSL_load_error_strings();
}

void
aug::setsslclient(conn_base& conn, sslctx& ctx)
{
    SSL* ssl = SSL_new(ctx);
    BIO* sbio = BIO_new_socket((int)aug_getosfd(conn.sfd().get()),
                               BIO_NOCLOSE);
    SSL_set_bio(ssl, sbio, sbio);
    SSL_set_app_data(ssl, &conn);
    aug_setsslclient(conn.sfd().get(), ssl);
}

void
aug::setsslserver(conn_base& conn, sslctx& ctx)
{
    SSL* ssl = SSL_new(ctx);
    BIO* sbio = BIO_new_socket((int)aug_getosfd(conn.sfd().get()),
                               BIO_NOCLOSE);
    SSL_set_bio(ssl, sbio, sbio);
    SSL_set_app_data(ssl, &conn);
    aug_setsslserver(conn.sfd().get(), ssl);
}

#endif // HAVE_OPENSSL_SSL_H
