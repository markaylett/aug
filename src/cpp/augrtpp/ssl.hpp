/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_SSL_HPP
#define AUGRTPP_SSL_HPP

#include "augconfig.h"

#if HAVE_OPENSSL_SSL_H

# include "augrtpp/config.hpp"
# include "augsyspp/exception.hpp"
# include "augsyspp/smartptr.hpp"
# include "augnet/ssl.h"

# include <openssl/ssl.h>

namespace aug {

    class conn_base;

    class ssl_error : public errinfo_error {
    public:
        ssl_error()
        {
        }
        ssl_error(const char* file, int line, unsigned long err)
        {
            aug_setsslerrinfo(cptr(*this), file, line, err);
        }
    };

    class AUGRTPP_API sslctx {

        SSL_CTX* const ctx_;

        sslctx(const sslctx&);

        sslctx&
        operator =(const sslctx&);

    public:
        ~sslctx() AUG_NOTHROW;

        sslctx();

        operator SSL_CTX*()
        {
            return ctx_;
        }
        SSL_CTX*
        get()
        {
            return ctx_;
        }
    };

    typedef smartptr<sslctx> sslctxptr;

    AUGRTPP_API void
    initssl();

    AUGRTPP_API void
    setsslclient(conn_base& conn, sslctx& ctx);

    AUGRTPP_API void
    setsslserver(conn_base& conn, sslctx& ctx);
}
#endif // HAVE_OPENSSL_SSL_H

#endif // AUGRTPP_SSL_HPP
