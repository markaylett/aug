/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_MAR_HPP
#define AUGNETPP_MAR_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/object.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/utility.hpp" // perrinfo()

#include "augnet/mar.h"

#include "augmar/mar.h"

#include <memory>               // auto_ptr<>

namespace aug {

    struct basic_marstatic {
    protected:
        ~basic_marstatic() AUG_NOTHROW
        {
        }
    public:
        static aug_mar_t
        create(aug_object* ob, const char* initial)
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
            create(aug_object* ob, const char* initial)
            {
                try {
                    return T::create(ob, initial);
                } AUG_SETERRINFOCATCH;
                return 0;
            }

            static int
            message(aug_object* ob, const char* initial, aug_mar_t mar)
            {
                try {
                    T::message(ob, initial, mar);
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
            create(aug_object* ob, const char* initial)
            {
                try {
                    return obtop<T*>(ob)->create(initial);
                } AUG_SETERRINFOCATCH;
                return 0;
            }

            static int
            message(aug_object* ob, const char* initial, aug_mar_t mar)
            {
                try {
                    obtop<T*>(ob)->message(initial, mar);
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
            if (marparser_ && -1 == aug_destroymarparser(marparser_))
                perrinfo(aug_tlx, "aug_destroymarparser() failed");
        }

        marparser(const null_&) AUG_NOTHROW
           : marparser_(0)
        {
        }

        marparser(mpoolref mpool, unsigned size,
                  const aug_marhandler& handler, objectref ob)
        {
            verify(marparser_
                   = aug_createmarparser(mpool.get(), size, &handler,
                                         ob.get()));
        }

        marparser(mpoolref mpool, unsigned size,
                  const aug_marhandler& handler, const null_&)
        {
            verify(marparser_
                   = aug_createmarparser(mpool.get(), size, &handler, 0));
        }

        template <typename T>
        marparser(mpoolref mpool, unsigned size, T& x)
        {
            aug::smartob<aug_boxptr> ob(createboxptr(mpool, &x, 0));
            verify(marparser_ = aug_createmarparser
                   (mpool.get(), size, &marnonstatic<T>(), ob.base()));
        }

        template <typename T>
        marparser(mpoolref mpool, unsigned size, std::auto_ptr<T>& x)
        {
            aug::smartob<aug_boxptr> ob(createboxptr(mpool, x));
            verify(marparser_ = aug_createmarparser
                   (mpool.get(), size, &marnonstatic<T>(), ob.base()));
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
