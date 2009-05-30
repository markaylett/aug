/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGNETPP_BASE64_HPP
#define AUGNETPP_BASE64_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/object.hpp"

#include "augctxpp/exception.hpp"

#include "augnet/base64.h"

#include "augctx/defs.h"        // AUG_MAXLINE

#include <memory>               // auto_ptr<>
#include <sstream>

namespace aug {

    template <aug_result (*T)(objectref, const char*, size_t)>
    aug_result
    base64cb(aug_object* ob, const char* buf, size_t len) AUG_NOTHROW
    {
        try {
            return T(ob, buf, len);
        } AUG_SETERRINFOCATCH;
        return AUG_FAILERROR;
    }

    template <typename T, aug_result (T::*U)(const char*, size_t)>
    aug_result
    base64memcb(aug_object* ob, const char* buf, size_t len) AUG_NOTHROW
    {
        try {
            return (obtop<T*>(ob)->*U)(buf, len);
        } AUG_SETERRINFOCATCH;
        return AUG_FAILERROR;
    }

    template <typename T>
    aug_result
    base64memcb(aug_object* ob, const char* buf, size_t len) AUG_NOTHROW
    {
        try {
            return obtop<T*>(ob)->base64cb(buf, len);
        } AUG_SETERRINFOCATCH;
        return AUG_FAILERROR;
    }

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

    class base64 : public mpool_ops {

        aug_base64_t base64_;

        base64(const base64&);

        base64&
        operator =(const base64&);

    public:
        ~base64() AUG_NOTHROW
        {
            if (base64_)
                aug_destroybase64(base64_);
        }

        base64(const null_&) AUG_NOTHROW
           : base64_(0)
        {
        }

        base64(mpoolref mpool, int mode, aug_base64cb_t cb,
               aug::obref<aug_object> ob)
        {
            verify(base64_ = aug_createbase64(mpool.get(), mode, cb,
                                              ob.get()));
        }

        base64(mpoolref mpool, int mode, aug_base64cb_t cb, const null_&)
        {
            verify(base64_ = aug_createbase64(mpool.get(), mode, cb, 0));
        }

        template <typename T>
        base64(mpoolref mpool, int mode, T& x)
        {
            aug::smartob<aug_boxptr> ob(createboxptr(mpool, &x, 0));
            verify(base64_ = aug_createbase64
                   (mpool.get(), mode, base64memcb<T>, ob.base()));
        }

        template <typename T>
        base64(mpoolref mpool, int mode, std::auto_ptr<T>& x)
        {
            aug::smartob<aug_boxptr> ob(createboxptr(mpool, x));
            verify(base64_ = aug_createbase64
                   (mpool.get(), mode, base64memcb<T>, ob.base()));
        }

        void
        swap(base64& rhs) AUG_NOTHROW
        {
            std::swap(base64_, rhs.base64_);
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
    swap(base64& lhs, base64& rhs) AUG_NOTHROW
    {
        lhs.swap(rhs);
    }

    namespace detail {
        inline aug_result
        base64os(objectref ob, const char* buf, size_t len)
        {
            std::ostream& os(*obtop<std::ostream*>(ob));
			os.write(buf, static_cast<std::streamsize>(len));
            return AUG_SUCCESS;
        }
        inline aug_result
        base64str(objectref ob, const char* buf, size_t len)
        {
            std::string& str(*obtop<std::string*>(ob));
            str.append(buf, static_cast<std::streamsize>(len));
            return AUG_SUCCESS;
        }
    }

    inline std::ostream&
    filterbase64(std::ostream& os, std::istream& is, int mode)
    {
        scoped_boxptr_wrapper<simple_boxptr> ob(&os);
        base64 b64(getmpool(aug_tlx), mode, base64cb<detail::base64os>, ob);
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
    filterbase64(const char* buf, size_t len, int mode)
    {
        std::string s;
        scoped_boxptr_wrapper<simple_boxptr> ob(&s);
        base64 b64(getmpool(aug_tlx), mode, base64cb<detail::base64str>, ob);
        appendbase64(b64, buf, len);
        finishbase64(b64);
        return s;
    }
}

inline bool
isnull(aug_base64_t base64)
{
    return !base64;
}

#endif // AUGNETPP_BASE64_HPP
