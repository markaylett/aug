/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_HTTP_HPP
#define AUGNETPP_HTTP_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/var.hpp"

#include "augsyspp/exception.hpp"

#include "augnet/http.h"

#include <memory> // auto_ptr<>

namespace aug {

    namespace detail {

        template <typename T>
        class httpstatic {
            static int
            initial(const aug_var* var, const char* value) AUG_NOTHROW
            {
                try {
                    T::initial(*var, value);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            field(const aug_var* var, const char* name,
                  const char* value) AUG_NOTHROW
            {
                try {
                    T::field(*var, name, value);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            csize(const aug_var* var, unsigned csize) AUG_NOTHROW
            {
                try {
                    T::csize(*var, csize);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            cdata(const aug_var* var, const void* cdata,
                  unsigned csize) AUG_NOTHROW
            {
                try {
                    T::cdata(*var, cdata, csize);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            end(const aug_var* var, int commit) AUG_NOTHROW
            {
                try {
                    T::end(*var, commit ? true : false);
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
            initial(const aug_var* var, const char* value) AUG_NOTHROW
            {
                try {
                    static_cast<T*>(var->arg_)->initial(value);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            field(const aug_var* var, const char* name,
                  const char* value) AUG_NOTHROW
            {
                try {
                    static_cast<T*>(var->arg_)->field(name, value);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            csize(const aug_var* var, unsigned csize) AUG_NOTHROW
            {
                try {
                    static_cast<T*>(var->arg_)->csize(csize);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            cdata(const aug_var* var, const void* cdata,
                  unsigned csize) AUG_NOTHROW
            {
                try {
                    static_cast<T*>(var->arg_)->cdata(cdata, csize);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            end(const aug_var* var, int commit) AUG_NOTHROW
            {
                try {
                    static_cast<T*>(var->arg_)->end(commit ? true : false);
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
                   const aug_var& var)
        {
            verify(httpparser_
                   = aug_createhttpparser(size, &handler, &var));
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
            aug_var var = { 0, &x };
            verify(httpparser_ = aug_createhttpparser
                   (size, &httpnonstatic<T>(), &var));
        }

        template <typename T>
        httpparser(unsigned size, std::auto_ptr<T>& x)
        {
            aug_var var;
            verify(httpparser_ = aug_createhttpparser
                   (size, &httpnonstatic<T>(),
                    &bindvar<deletearg<T> >(var, *x)));
            x.release();
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
