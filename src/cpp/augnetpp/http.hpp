/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_HTTP_HPP
#define AUGNETPP_HTTP_HPP

#include "augnetpp/config.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/mpool.hpp"

#include "augnet/http.h"

namespace aug {

    class httpparser : public mpool_ops {

        aug_httpparser_t httpparser_;

        httpparser(const httpparser&);

        httpparser&
        operator =(const httpparser&);

    public:
        ~httpparser() AUG_NOTHROW
        {
            if (httpparser_)
                aug_destroyhttpparser(httpparser_);
        }

        httpparser(const null_&) AUG_NOTHROW
           : httpparser_(0)
        {
        }

        httpparser(mpoolref mpool, httphandlerref httphandler,
                   unsigned size = 0)
            : httpparser_(aug_createhttpparser(mpool.get(), httphandler.get(),
                                               size))
        {
            verify(httpparser_);
        }

        void
        swap(httpparser& rhs) AUG_NOTHROW
        {
            std::swap(httpparser_, rhs.httpparser_);
        }

        operator aug_httpparser_t()
        {
            return httpparser_;
        }

        aug_httpparser_t
        get()
        {
            return httpparser_;
        }
    };

    inline void
    swap(httpparser& lhs, httpparser& rhs) AUG_NOTHROW
    {
        lhs.swap(rhs);
    }

    inline void
    appendhttp(aug_httpparser_t parser, const char* buf, unsigned size)
    {
        verify(aug_appendhttp(parser, buf, size));
    }

    inline void
    finishhttp(aug_httpparser_t parser)
    {
        verify(aug_finishhttp(parser));
    }
}

#endif // AUGNETPP_HTTP_HPP
