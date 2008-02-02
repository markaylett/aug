/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_HTTP_HPP
#define AUGNETPP_HTTP_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/object.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/utility.hpp"

#include "augnet/http.h"

#include <memory> // auto_ptr<>

namespace aug {

    namespace detail {

        template <typename T>
        class httpstatic {
            static int
            initial(aug_object* ob, const char* value) AUG_NOTHROW
            {
                try {
                    T::initial(ob, value);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            field(aug_object* ob, const char* name,
                  const char* value) AUG_NOTHROW
            {
                try {
                    T::field(ob, name, value);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            csize(aug_object* ob, unsigned csize) AUG_NOTHROW
            {
                try {
                    T::csize(ob, csize);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            cdata(aug_object* ob, const void* cdata,
                  unsigned csize) AUG_NOTHROW
            {
                try {
                    T::cdata(ob, cdata, csize);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            end(aug_object* ob, int commit) AUG_NOTHROW
            {
                try {
                    T::end(ob, commit ? true : false);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
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
            static int
            initial(aug_object* ob, const char* value) AUG_NOTHROW
            {
                try {
                    obtoaddr<T*>(ob)->initial(value);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            field(aug_object* ob, const char* name,
                  const char* value) AUG_NOTHROW
            {
                try {
                    obtoaddr<T*>(ob)->field(name, value);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            csize(aug_object* ob, unsigned csize) AUG_NOTHROW
            {
                try {
                    obtoaddr<T*>(ob)->csize(csize);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            cdata(aug_object* ob, const void* cdata,
                  unsigned csize) AUG_NOTHROW
            {
                try {
                    obtoaddr<T*>(ob)->cdata(cdata, csize);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            end(aug_object* ob, int commit) AUG_NOTHROW
            {
                try {
                    obtoaddr<T*>(ob)->end(commit ? true : false);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
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
            if (-1 == aug_destroyhttpparser(httpparser_))
                perrinfo("aug_destroyhttpparser() failed");
        }

        httpparser(unsigned size, const aug_httphandler& handler,
                   aug_object* ob)
        {
            verify(httpparser_
                   = aug_createhttpparser(size, &handler, ob));
        }

        httpparser(unsigned size, const aug_httphandler& handler,
                   const null_&)
        {
            verify(httpparser_
                   = aug_createhttpparser(size, &handler, 0));
        }

        template <typename T>
        httpparser(unsigned size, T& x)
        {
            aug::smartob<aug_addrob> ob(createaddrob(&x, 0));
            verify(httpparser_ = aug_createhttpparser
                   (size, &httpnonstatic<T>(), ob.base()));
        }

        template <typename T>
        httpparser(unsigned size, std::auto_ptr<T>& x)
        {
            aug::smartob<aug_addrob> ob(createaddrob(x));
            verify(httpparser_ = aug_createhttpparser
                   (size, &httpnonstatic<T>(), ob.base()));
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
