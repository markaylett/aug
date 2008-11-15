/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGASPP_SSL_HPP
#define AUGASPP_SSL_HPP

#include "augaspp/config.hpp"

#include "augsyspp/smartptr.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/mpool.hpp"

#include "augnet/ssl.h"

struct ssl_ctx_st;

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

    class AUGASPP_API sslctx : public mpool_ops {

        ssl_ctx_st* const ctx_;

        sslctx(const sslctx&);

        sslctx&
        operator =(const sslctx&);

    public:
        ~sslctx() AUG_NOTHROW;

        sslctx();

        operator ssl_ctx_st*()
        {
            return ctx_;
        }
        ssl_ctx_st*
        get()
        {
            return ctx_;
        }
    };

    typedef smartptr<sslctx> sslctxptr;

    AUGASPP_API void
    initssl();
}

#endif // AUGASPP_SSL_HPP
