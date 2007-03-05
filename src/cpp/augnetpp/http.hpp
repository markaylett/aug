/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_HTTP_HPP
#define AUGNETPP_HTTP_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/var.hpp"

#include "augsyspp/exception.hpp"

#include "augnet/http.h"

namespace aug {

    class httphandler_base {

        virtual void
        do_initial(const char* value) = 0;

        virtual void
        do_field(const char* name, const char* value) = 0;

        virtual void
        do_csize(unsigned csize) = 0;

        virtual void
        do_cdata(const void* cdata, unsigned csize) = 0;

        virtual void
        do_end(bool commit) = 0;

    public:
        virtual
        ~httphandler_base() AUG_NOTHROW
        {
        }
        void
        initial(const char* value)
        {
            do_initial(value);
        }
        void
        field(const char* name, const char* value)
        {
            do_field(name, value);
        }
        void
        csize(unsigned csize)
        {
            do_csize(csize);
        }
        void
        cdata(const void* cdata, unsigned csize)
        {
            do_cdata(cdata, csize);
        }
        void
        end(bool commit)
        {
        }
    };

    namespace detail {

        inline void
        initial(const aug_var* arg, const char* value)
        {
            try {
                httphandler_base* ptr = static_cast<
                    httphandler_base*>(aug_getvarp(arg));
                ptr->initial(value);
            } AUG_SETERRINFOCATCH;
        }

        inline void
        field(const aug_var* arg, const char* name, const char* value)
        {
            try {
                httphandler_base* ptr = static_cast<
                    httphandler_base*>(aug_getvarp(arg));
                ptr->field(name, value);
            } AUG_SETERRINFOCATCH;
        }

        inline void
        csize(const aug_var* arg, unsigned csize)
        {
            try {
                httphandler_base* ptr = static_cast<
                    httphandler_base*>(aug_getvarp(arg));
                ptr->csize(csize);
            } AUG_SETERRINFOCATCH;
        }

        inline void
        cdata(const aug_var* arg, const void* cdata, unsigned csize)
        {
            try {
                httphandler_base* ptr = static_cast<
                    httphandler_base*>(aug_getvarp(arg));
                ptr->cdata(cdata, csize);
            } AUG_SETERRINFOCATCH;
        }

        inline void
        end(const aug_var* arg, int commit)
        {
            try {
                httphandler_base* ptr = static_cast<
                    httphandler_base*>(aug_getvarp(arg));
                ptr->end(commit ? true : false);
            } AUG_SETERRINFOCATCH;
        }
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

        httpparser(unsigned size, httphandler_base& handler)
        {
            static const aug_httphandler local = {
                detail::initial,
                detail::field,
                detail::csize,
                detail::cdata,
                detail::end
            };
            var v(this);
            verify(httpparser_
                   = aug_createhttpparser(size, &local, cptr(v)));
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
    parsehttp(aug_httpparser_t parser, const char* buf, unsigned size)
    {
        verify(aug_parsehttp(parser, buf, size));
    }

    inline void
    endhttp(aug_httpparser_t parser)
    {
        verify(aug_endhttp(parser));
    }
}

#endif // AUGNETPP_HTTP_HPP
