/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_HTTP_HPP
#define AUGNETPP_HTTP_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/object.hpp"

#include "augctxpp/exception.hpp"

#include "augnet/http.h"

#include <memory> // auto_ptr<>

namespace aug {

    namespace detail {

        template <typename T>
        class httpstatic {
            static aug_result
            initial(aug_object* ob, const char* value) AUG_NOTHROW
            {
                try {
                    return T::initial(ob, value);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }
            static aug_result
            field(aug_object* ob, const char* name,
                  const char* value) AUG_NOTHROW
            {
                try {
                    return T::field(ob, name, value);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }
            static aug_result
            csize(aug_object* ob, unsigned csize) AUG_NOTHROW
            {
                try {
                    return T::csize(ob, csize);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;

            }
            static aug_result
            cdata(aug_object* ob, const void* cdata,
                  unsigned csize) AUG_NOTHROW
            {
                try {
                    return T::cdata(ob, cdata, csize);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }
            static aug_result
            end(aug_object* ob, int commit) AUG_NOTHROW
            {
                try {
                    return T::end(ob, commit ? true : false);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }

        public:
            static const aug_httphandler&
            get()
            {
                static const aug_httphandler local = {
                    initial,
                    field,
                    csize,
                    cdata,
                    end
                };
                return local;
            }
        };

        template <typename T>
        class httpnonstatic {
            static aug_result
            initial(aug_object* ob, const char* value) AUG_NOTHROW
            {
                try {
                    return obtop<T*>(ob)->initial(value);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }
            static aug_result
            field(aug_object* ob, const char* name,
                  const char* value) AUG_NOTHROW
            {
                try {
                    return obtop<T*>(ob)->field(name, value);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }
            static aug_result
            csize(aug_object* ob, unsigned csize) AUG_NOTHROW
            {
                try {
                    return obtop<T*>(ob)->csize(csize);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }
            static aug_result
            cdata(aug_object* ob, const void* cdata,
                  unsigned csize) AUG_NOTHROW
            {
                try {
                    return obtop<T*>(ob)->cdata(cdata, csize);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }
            static aug_result
            end(aug_object* ob, int commit) AUG_NOTHROW
            {
                try {
                    return obtop<T*>(ob)->end(commit ? true : false);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }

        public:
            static const aug_httphandler&
            get()
            {
                static const aug_httphandler local = {
                    initial,
                    field,
                    csize,
                    cdata,
                    end
                };
                return local;
            }
        };
    }

    template <typename T>
    const aug_httphandler&
    httpstatic()
    {
        return detail::httpstatic<T>::get();
    }

    template <typename T>
    const aug_httphandler&
    httpnonstatic()
    {
        return detail::httpnonstatic<T>::get();
    }

    class httpparser {

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

        httpparser(mpoolref mpool, unsigned size,
                   const aug_httphandler& handler, objectref ob)
        {
            verify(httpparser_
                   = aug_createhttpparser(mpool.get(), size, &handler,
                                          ob.get()));
        }

        httpparser(mpoolref mpool, unsigned size,
                   const aug_httphandler& handler, const null_&)
        {
            verify(httpparser_
                   = aug_createhttpparser(mpool.get(), size, &handler, 0));
        }

        template <typename T>
        httpparser(mpoolref mpool, unsigned size, T& x)
        {
            aug::smartob<aug_boxptr> ob(createboxptr(mpool, &x, 0));
            verify(httpparser_ = aug_createhttpparser
                   (mpool.get(), size, &httpnonstatic<T>(), ob.base()));
        }

        template <typename T>
        httpparser(mpoolref mpool, unsigned size, std::auto_ptr<T>& x)
        {
            aug::smartob<aug_boxptr> ob(createboxptr(mpool, x));
            verify(httpparser_ = aug_createhttpparser
                   (mpool.get(), size, &httpnonstatic<T>(), ob.base()));
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
