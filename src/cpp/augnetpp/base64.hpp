/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_BASE64_HPP
#define AUGNETPP_BASE64_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/var.hpp"

#include "augsyspp/exception.hpp"

#include "augnet/base64.h"

#include <sstream>

namespace aug {

    template <void (*T)(const aug_var&, const char*, size_t)>
    int
    base64cb(const aug_var* var, const char* buf, size_t len) AUG_NOTHROW
    {
        try {
            T(*var, buf, len);
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

        base64(aug_base64mode mode, aug_base64cb_t cb, const aug_var& var)
        {
            verify(base64_ = aug_createbase64(mode, cb, &var));
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
        base64os(const aug_var& var, const char* buf, size_t len)
        {
            std::ostream& os(*static_cast<std::ostream*>(var.arg_));
			os.write(buf, static_cast<std::streamsize>(len));
        }
        inline void
        base64str(const aug_var& var, const char* buf, size_t len)
        {
            std::string& str(*static_cast<std::string*>(var.arg_));
            str.append(buf, static_cast<std::streamsize>(len));
        }
    }

    inline std::ostream&
    filterbase64(std::ostream& os, std::istream& is, aug_base64mode mode)
    {
        aug_var var = { 0, &os };
        base64 b64(mode, base64cb<detail::base64os>, var);
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
        aug_var var = { 0, &s };
        base64 b64(mode, base64cb<detail::base64str>, var);
        appendbase64(b64, buf, len);
        finishbase64(b64);
        return s;
    }
}

#endif // AUGNETPP_BASE64_HPP
