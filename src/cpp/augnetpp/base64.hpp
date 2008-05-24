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

#include "augctx/defs.h" // AUG_MAXLINE

#include <memory>        // auto_ptr<>
#include <sstream>

namespace aug {

    template <void (*T)(objectref, const char*, size_t)>
    int
    base64cb(aug_object* ob, const char* buf, size_t len) AUG_NOTHROW
    {
        try {
            T(ob, buf, len);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    template <typename T, void (T::*U)(const char*, size_t)>
    int
    base64memcb(aug_object* ob, const char* buf, size_t len) AUG_NOTHROW
    {
        try {
            (obtoaddr<T*>(ob)->*U)(buf, len);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    template <typename T>
    int
    base64memcb(aug_object* ob, const char* buf, size_t len) AUG_NOTHROW
    {
        try {
            obtoaddr<T*>(ob)->base64cb(buf, len);
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
                perrinfo(aug_tlx, "aug_destroybase64() failed");
        }

        base64(aug_base64mode mode, aug_base64cb_t cb,
               aug::obref<aug_object> ob)
        {
            verify(base64_ = aug_createbase64(mode, cb, ob.get()));
        }

        base64(aug_base64mode mode, aug_base64cb_t cb, const null_&)
        {
            verify(base64_ = aug_createbase64(mode, cb, 0));
        }

        template <typename T>
        base64(aug_base64mode mode, T& x)
        {
            aug::smartob<aug_addrob> ob(createaddrob(&x, 0));
            verify(base64_ = aug_createbase64
                   (mode, base64memcb<T>, ob.base()));
        }

        template <typename T>
        base64(aug_base64mode mode, std::auto_ptr<T>& x)
        {
            aug::smartob<aug_addrob> ob(createaddrob(x));
            verify(base64_ = aug_createbase64
                   (mode, base64memcb<T>, ob.base()));
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
        base64os(objectref ob, const char* buf, size_t len)
        {
            std::ostream& os(*obtoaddr<std::ostream*>(ob));
			os.write(buf, static_cast<std::streamsize>(len));
        }
        inline void
        base64str(objectref ob, const char* buf, size_t len)
        {
            std::string& str(*obtoaddr<std::string*>(ob));
            str.append(buf, static_cast<std::streamsize>(len));
        }
    }

    inline std::ostream&
    filterbase64(std::ostream& os, std::istream& is, aug_base64mode mode)
    {
        scoped_addrob<simple_addrob> ob(&os);
        base64 b64(mode, base64cb<detail::base64os>, ob);
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
        scoped_addrob<simple_addrob> ob(&s);
        base64 b64(mode, base64cb<detail::base64str>, ob);
        appendbase64(b64, buf, len);
        finishbase64(b64);
        return s;
    }
}

#endif // AUGNETPP_BASE64_HPP
