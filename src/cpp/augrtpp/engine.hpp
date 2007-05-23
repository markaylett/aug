/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_ENGINE_HPP
#define AUGRTPP_ENGINE_HPP

#include "augrtpp/config.hpp"

namespace aug {

    namespace detail {
        struct engineimpl;
    }

    class engine {

        detail::engineimpl* const impl_;

        engine(const engine& rhs);

        engine&
        operator =(const engine& rhs);

    public:
        ~engine() AUG_NOTHROW;

        engine();

        bool
        stop() const;
    };
}

#endif // AUGRTPP_ENGINE_HPP
