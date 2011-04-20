/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGNETPP_HTTP_HPP
#define AUGNETPP_HTTP_HPP

#include "augnetpp/config.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/mpool.hpp"

#include "augnet/http.h"

namespace aug {


    inline void
    appendhttp_BIN(aug_httpparser_t parser, const char* buf, unsigned size)
    {
        verify(aug_appendhttp_BIN(parser, buf, size));
    }

    inline void
    finishhttp_BIN(aug_httpparser_t parser)
    {
        verify(aug_finishhttp_BIN(parser));
    }

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
}

inline bool
isnull(aug_httpparser_t httpparser)
{
    return !httpparser;
}

#endif // AUGNETPP_HTTP_HPP
