/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_MAR_HPP
#define AUGNETPP_MAR_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/object.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/utility.hpp"

#include "augnet/mar.h"

#include "augmar/mar.h"

#include <memory> // auto_ptr<>

namespace aug {

    struct basic_marstatic {
    protected:
        ~basic_marstatic() AUG_NOTHROW
        {
        }
    public:
        static aug_mar_t
        create(aug_object* user, const char* initial)
        {
            return aug_createmar();
        }
    };

    struct basic_marnonstatic {
    protected:
        ~basic_marnonstatic() AUG_NOTHROW
        {
        }
    public:
        aug_mar_t
        create(const char* initial)
        {
            return aug_createmar();
        }
    };

    namespace detail {

        template <typename T>
        class marstatic {

            static aug_mar_t
            create(aug_object* user, const char* initial)
            {
                try {
                    return T::create(user, initial);
                } AUG_SETERRINFOCATCH;
                return 0;
            }

            static int
            message(aug_object* user, const char* initial, aug_mar_t mar)
            {
                try {
                    T::message(user, initial, mar);
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

        template <typename T>
        class marnonstatic {

            static aug_mar_t
            create(aug_object* user, const char* initial)
            {
                try {
                    return obtoaddr<T*>(user)->create(initial);
                } AUG_SETERRINFOCATCH;
                return 0;
            }

            static int
            message(aug_object* user, const char* initial, aug_mar_t mar)
            {
                try {
                    obtoaddr<T*>(user)->message(initial, mar);
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
    marstatic()
    {
        return detail::marstatic<T>::get();
    }

    template <typename T>
    const aug_marhandler&
    marnonstatic()
    {
        return detail::marnonstatic<T>::get();
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
                  aug_object* user)
        {
            verify(marparser_
                   = aug_createmarparser(size, &handler, user));
        }

        marparser(unsigned size, const aug_marhandler& handler, const null_&)
        {
            verify(marparser_
                   = aug_createmarparser(size, &handler, 0));
        }

        template <typename T>
        marparser(unsigned size, T& x)
        {
            smartob<aug_addrob> obj(createaddrob(&x, 0));
            verify(marparser_ = aug_createmarparser
                   (size, &marnonstatic<T>(), obj.base()));
        }

        template <typename T>
        marparser(unsigned size, std::auto_ptr<T>& x)
        {
            smartob<aug_addrob> obj(createaddrob(x));
            verify(marparser_ = aug_createmarparser
                   (size, &marnonstatic<T>(), obj.base()));
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
