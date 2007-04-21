/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_SSL_HPP
#define AUGAS_SSL_HPP

#if !defined(_MSC_VER)
# include "augconfig.h"
#endif // !_MSC_VER

#if HAVE_OPENSSL_SSL_H

# include "augsyspp/exception.hpp"
# include "augsyspp/smartptr.hpp"
# include "augnet/ssl.h"

# include <map>

# include <openssl/ssl.h>

namespace augas {

    class options;

    class ssl_error : public aug::errinfo_error {
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

        SSL_CTX* const ctx_;

        sslctx(const sslctx&);

        sslctx&
        operator =(const sslctx&);

    public:
        ~sslctx() AUG_NOTHROW;

        sslctx(const char* certfile, const char* keyfile,
               const char* password, const char* cafile);

        void
        setsslclient(aug::fdref ref);

        void
        setsslserver(aug::fdref ref);
    };

    typedef aug::smartptr<sslctx> sslctxptr;
    typedef std::map<std::string, sslctxptr> sslctxs;

    void
    initssl();

    void
    loadsslctxs(sslctxs& sslctxs, const options& options);
}
#endif // HAVE_OPENSSL_SSL_H

#endif // AUGAS_SSL_HPP
