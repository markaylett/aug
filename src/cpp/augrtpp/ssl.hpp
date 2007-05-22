/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_SSL_HPP
#define AUGRTPP_SSL_HPP

#include "augconfig.h"

#if HAVE_OPENSSL_SSL_H

# include "augsyspp/exception.hpp"
# include "augsyspp/smartptr.hpp"
# include "augnet/ssl.h"

# include <map>

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

    class sslctx {

        friend void
        setclient(sslctx& ctx, conn_base& conn);

        friend void
        setserver(sslctx& ctx, conn_base& conn);

        SSL_CTX* const ctx_;

        sslctx(const sslctx&);

        sslctx&
        operator =(const sslctx&);

    public:
        ~sslctx() AUG_NOTHROW;

        sslctx();
    };

    typedef smartptr<sslctx> sslctxptr;
    typedef std::map<std::string, sslctxptr> sslctxs;

    void
    initssl();

    void
    setclient(sslctx& ctx, conn_base& conn);

    void
    setserver(sslctx& ctx, conn_base& conn);
}
#endif // HAVE_OPENSSL_SSL_H

#endif // AUGRTPP_SSL_HPP
