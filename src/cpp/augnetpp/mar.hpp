/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_MAR_HPP
#define AUGNETPP_MAR_HPP

#include "augnetpp/config.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/mpool.hpp"

#include "augnet/mar.h"

namespace aug {

    class marparser : public mpool_ops {

        aug_marparser_t marparser_;

        marparser(const marparser&);

        marparser&
        operator =(const marparser&);

    public:
        ~marparser() AUG_NOTHROW
        {
            if (marparser_)
                aug_destroymarparser(marparser_);
        }

        marparser(const null_&) AUG_NOTHROW
           : marparser_(0)
        {
        }

        marparser(mpoolref mpool, marpoolref marpool, unsigned size = 0)
           : marparser_(aug_createmarparser(mpool.get(), marpool.get(), size))
        {
            verify(marparser_);
        }

        void
        swap(marparser& rhs) AUG_NOTHROW
        {
            std::swap(marparser_, rhs.marparser_);
        }

        operator aug_marparser_t()
        {
            return marparser_;
        }

        aug_marparser_t
        get()
        {
            return marparser_;
        }
    };

    inline void
    swap(marparser& lhs, marparser& rhs) AUG_NOTHROW
    {
        lhs.swap(rhs);
    }

    inline void
    appendmar(aug_marparser_t parser, const char* buf, unsigned size)
    {
        verify(aug_appendmar(parser, buf, size));
    }

    inline void
    finishmar(aug_marparser_t parser)
    {
        verify(aug_finishmar(parser));
    }
}

#endif // AUGNETPP_MAR_HPP
