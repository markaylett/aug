/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_MAR_HPP
#define AUGNETPP_MAR_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/var.hpp"

#include "augsyspp/exception.hpp"

#include "augnet/mar.h"

#include "augmar/mar.h"

namespace aug {

    struct basic_marhandler {
    protected:
        ~basic_marhandler() AUG_NOTHROW
        {
        }
    public:
        static aug_mar_t
        create(const aug_var& var, const char* initial)
        {
            return aug_createmar();
        }
    };

    namespace detail {

        template <typename T>
        class marhandler {

            static aug_mar_t
            create(const aug_var* var, const char* initial)
            {
                try {
                    return T::create(*var, initial);
                } AUG_SETERRINFOCATCH;
                return 0;
            }

            static int
            message(const aug_var* var, const char* initial, aug_mar_t mar)
            {
                try {
                    T::message(*var, initial, mar);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }

        public:
            static const aug_marhandler&
            get()
            {
                static const aug_marhandler local = {
                    create,
                    message
                };
                return local;
            }
        };
    }

    template <typename T>
    const aug_marhandler&
    marhandler()
    {
        return detail::marhandler<T>::get();
    }

    class marparser {

        aug_marparser_t marparser_;

        marparser(const marparser&);

        marparser&
        operator =(const marparser&);

    public:
        ~marparser() AUG_NOTHROW
        {
            if (-1 == aug_destroymarparser(marparser_))
                perrinfo("aug_destroymarparser() failed");
        }

        marparser(unsigned size, const aug_marhandler& handler,
                   const aug_var& var)
        {
            verify(marparser_
                   = aug_createmarparser(size, &handler, &var));
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
    parsemar(aug_marparser_t parser, const char* buf, unsigned size)
    {
        verify(aug_parsemar(parser, buf, size));
    }

    inline void
    endmar(aug_marparser_t parser)
    {
        verify(aug_endmar(parser));
    }
}

#endif // AUGNETPP_MAR_HPP
