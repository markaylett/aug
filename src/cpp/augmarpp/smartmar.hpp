/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
 * \file smartmar.hpp
 * \brief TODO
 */

#ifndef AUGMARPP_SMARTMAR_HPP
#define AUGMARPP_SMARTMAR_HPP

#include "augmarpp/types.hpp"

namespace aug {

    class AUGMARPP_API smartmar {
        marref ref_;

        explicit
        smartmar(marref ref, bool retain);

    public:
        ~smartmar() NOTHROW;

        smartmar(const null_&) NOTHROW
            : ref_(null)
        {
        }

        smartmar(const smartmar& rhs);

        smartmar&
        operator =(const smartmar& rhs);

        void
        swap(smartmar& rhs) NOTHROW;

        void
        release();

        static smartmar
        attach(aug_mar_t mar)
        {
            return smartmar(mar, false);
        }

        static smartmar
        retain(aug_mar_t mar)
        {
            return smartmar(mar, true);
        }

        operator marref() const
        {
            return ref_;
        }
        aug_mar_t
        get() const
        {
            return ref_.get();
        }
    };
}

#endif // AUGMARPP_SMARTMAR_HPP
