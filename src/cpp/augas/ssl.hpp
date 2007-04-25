/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_SSL_HPP
#define AUGAS_SSL_HPP

#include "augconfig.h"

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

        friend aug::smartptr<sslctx>
        createsslctx(const std::string& name, const options& options);

        SSL_CTX* const ctx_;

        sslctx(const sslctx&);

        sslctx&
        operator =(const sslctx&);

        sslctx();

    public:
        ~sslctx() AUG_NOTHROW;

        void
        setclient(aug::fdref ref);

        void
        setserver(aug::fdref ref);
    };

    typedef aug::smartptr<sslctx> sslctxptr;
    typedef std::map<std::string, sslctxptr> sslctxs;

    void
    initssl();

    sslctxptr
    createsslctx(const std::string& name, const options& options);

    void
    createsslctxs(sslctxs& sslctxs, const options& options);
}
#endif // HAVE_OPENSSL_SSL_H

#endif // AUGAS_SSL_HPP
