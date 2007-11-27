/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_BASE64_HPP
#define AUGNETPP_BASE64_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/object.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/utility.hpp"

#include "augnet/base64.h"

#include <memory> // auto_ptr<>
#include <sstream>

namespace aug {

    template <void (*T)(aug_object*, const char*, size_t)>
    int
    base64cb(aug_object* user, const char* buf, size_t len) AUG_NOTHROW
    {
        try {
            T(user, buf, len);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    template <typename T, void (T::*U)(const char*, size_t)>
    int
    base64memcb(aug_object* user, const char* buf, size_t len) AUG_NOTHROW
    {
        try {
            (static_cast<T*>(aug_obtoaddr(user))->*U)(buf, len);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    template <typename T>
    int
    base64memcb(aug_object* user, const char* buf, size_t len) AUG_NOTHROW
    {
        try {
            static_cast<T*>(aug_obtoaddr(user))->base64cb(buf, len);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    class base64 {

        aug_base64_t base64_;

        base64(const base64&);

        base64&
        operator =(const base64&);

    public:
        ~base64() AUG_NOTHROW
        {
            if (-1 == aug_destroybase64(base64_))
                perrinfo("aug_destroybase64() failed");
        }

        base64(aug_base64mode mode, aug_base64cb_t cb, aug_object* user)
        {
            verify(base64_ = aug_createbase64(mode, cb, user));
        }

        base64(aug_base64mode mode, aug_base64cb_t cb, const null_&)
        {
            verify(base64_ = aug_createbase64(mode, cb, 0));
        }

        template <typename T>
        base64(aug_base64mode mode, T& x)
        {
            scoped_addrob obj(&x);
            verify(base64_
                   = aug_createbase64(mode, base64memcb<T>, &obj));
        }

        template <typename T>
        base64(aug_base64mode mode, std::auto_ptr<T>& x)
        {
            scoped_addrob obj(x.release(), deleter<T>);
            verify(base64_
                   = aug_createbase64(mode, base64memcb<T>, obj));
        }

        operator aug_base64_t()
        {
            return base64_;
        }

        aug_base64_t
        get()
        {
            return base64_;
        }
    };

    inline void
    appendbase64(aug_base64_t base64, const char* buf, size_t size)
    {
        verify(aug_appendbase64(base64, buf, size));
    }

    inline void
    finishbase64(aug_base64_t base64)
    {
        verify(aug_finishbase64(base64));
    }

    namespace detail {
        inline void
        base64os(aug_object* user, const char* buf, size_t len)
        {
            std::ostream& os(*obtoaddr<std::ostream*>(user));
			os.write(buf, static_cast<std::streamsize>(len));
        }
        inline void
        base64str(aug_object* user, const char* buf, size_t len)
        {
            std::string& str(*obtoaddr<std::string*>(user));
            str.append(buf, static_cast<std::streamsize>(len));
        }
    }

    inline std::ostream&
    filterbase64(std::ostream& os, std::istream& is, aug_base64mode mode)
    {
        scoped_addrob obj(&os, 0);
        base64 b64(mode, base64cb<detail::base64os>, obj.object());
        char buf[AUG_MAXLINE];
        do {
            is.read(buf, sizeof(buf));
            if (is.gcount())
                appendbase64(b64, buf, is.gcount());
        } while (is);
        finishbase64(b64);
        return os;
    }

    inline std::string
    filterbase64(const char* buf, size_t len, aug_base64mode mode)
    {
        std::string s;
        scoped_addrob obj(&s, 0);
        base64 b64(mode, base64cb<detail::base64str>, obj.object());
        appendbase64(b64, buf, len);
        finishbase64(b64);
        return s;
    }
}

#endif // AUGNETPP_BASE64_HPP
