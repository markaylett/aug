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
#ifndef AUGABIPP_HPP
#define AUGABIPP_HPP

#include "augnullpp.hpp"

#include "augabi.h"

#include <algorithm> // swap()
#include <cassert>
#include <cstring>   // strcmp()

#if !defined(AUG_NOTHROW)
# if !defined(NDEBUG)
#  define AUG_NOTHROW throw()
# else /* NDEBUG */
#  define AUG_NOTHROW
# endif /* NDEBUG */
#endif /* !AUG_NOTHROW */

#define AUG_OBJECTASSERT(T) \
        do { (void)sizeof(aug::object_traits<T>::vtbl); } while (0)

namespace aug {

    template <typename T>
    struct object_traits;

    template <>
    struct object_traits<aug_object> {
        typedef aug_objectvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_objectid;
        }
    };

    template <typename T>
    class obref;

    /**
     * Specialization for base object.
     */

    template <>
    class obref<aug_object> {
    public:
        typedef aug_object obtype;
        typedef object_traits<aug_object>::vtbl vtbl;
    protected:
        void* ptr_;
        obref(void* ptr) AUG_NOTHROW
            : ptr_(ptr)
        {
        }
    public:
        obref(const null_&) AUG_NOTHROW
            : ptr_(0)
        {
        }
        obref(aug_object* ptr) AUG_NOTHROW
            : ptr_(ptr)
        {
        }
        aug_object*
        base() const AUG_NOTHROW
        {
            return static_cast<aug_object*>(ptr_);
        }
        aug_object*
        get() const AUG_NOTHROW
        {
            return base();
        }
    };

    typedef obref<aug_object> objectref;

    /**
     * Generalized implementation is derived from base.
     *
     * Slicing allows conversion to base.
     */

    template <typename T>
    class obref : public objectref {
        typedef T type;
        typedef typename object_traits<T>::vtbl vtbl;
    public:
        obref(const null_&) AUG_NOTHROW
            : objectref(null)
        {
        }
        obref(T* ptr) AUG_NOTHROW
            : objectref(ptr)
        {
        }
        T*
        get() const AUG_NOTHROW
        {
            return static_cast<T*>(ptr_);
        }
    };

    template <typename T, typename U>
    bool
    operator ==(obref<T> lhs, obref<U> rhs)
    {
        return lhs.get() == rhs.get();
    }

    template <typename T, typename U>
    bool
    operator !=(obref<T> lhs, obref<U> rhs)
    {
        return lhs.get() == rhs.get();
    }

    /**
     * Argument must not be null.
     */

    template <typename T>
    void
    retain(obref<T> ref) AUG_NOTHROW
    {
        assert(ref.get());
        ref.get()->vtbl_->retain_(ref.get());
    }

    /**
     * Argument must not be null.
     */

    template <typename T>
    T*
    retget(const obref<T>& ref) AUG_NOTHROW
    {
        if (ref != null)
            retain(ref);
        return ref.get();
    }

    /**
     * Argument must not be null.
     */

    template <typename T>
    void
    release(obref<T> ref) AUG_NOTHROW
    {
        assert(ref.get());
        ref.get()->vtbl_->release_(ref.get());
    }

    template <typename T>
    obref<T>
    obptr(T* ptr) AUG_NOTHROW
    {
        return obref<T>(ptr);
    }

    template <typename T>
    class smartob {
    public:
        typedef T obtype;
    private:
        obref<T> ref_;

        smartob(obref<T> ref, bool ret) AUG_NOTHROW
            : ref_(ref)
        {
            if (null != ref && ret)
                aug::retain(ref);
        }

    public:
        ~smartob() AUG_NOTHROW
        {
            if (null != ref_)
                aug::release(ref_);
        }
        smartob(const null_&) AUG_NOTHROW
            : ref_(null)
        {
        }
        smartob(const smartob& rhs) AUG_NOTHROW
            : ref_(rhs.ref_)
        {
            if (null != ref_)
                aug::retain(ref_);
        }
        smartob&
        operator =(const null_&) AUG_NOTHROW
        {
            *this = smartob(null);
            return *this;
        }
        smartob&
        operator =(const smartob& rhs) AUG_NOTHROW
        {
            smartob tmp(rhs);
            swap(tmp);
            return *this;
        }
        void
        swap(smartob& rhs) AUG_NOTHROW
        {
            std::swap(ref_, rhs.ref_);
        }
        static smartob
        attach(obref<T> ref) AUG_NOTHROW
        {
            return smartob(ref, false);
        }
        static smartob
        retain(obref<T> ref) AUG_NOTHROW
        {
            return smartob(ref, true);
        }
        aug_object*
        base() const AUG_NOTHROW
        {
            return obref<aug_object>(ref_).get();
        }
        T*
        get() const AUG_NOTHROW
        {
            return ref_.get();
        }
        operator smartob<aug_object>() const AUG_NOTHROW
        {
            return smartob<aug_object>::retain(ref_);
        }
        operator obref<T>() const AUG_NOTHROW
        {
            return ref_;
        }
    };

    template <typename T, typename U>
    bool
    operator ==(const smartob<T>& lhs, const smartob<U>& rhs)
    {
        return lhs.get() == rhs.get();
    }

    template <typename T, typename U>
    bool
    operator !=(const smartob<T>& lhs, const smartob<U>& rhs)
    {
        return lhs.get() == rhs.get();
    }

    /**
     * Argument may be null.
     */

    template <typename T>
    void
    retain(const smartob<T>& sob) AUG_NOTHROW
    {
        retain<T>(static_cast<obref<T> >(sob));
    }

    template <typename T>
    T*
    retget(const smartob<T>& sob) AUG_NOTHROW
    {
        return retget<T>(static_cast<obref<T> >(sob));
    }

    template <typename T>
    void
    release(const smartob<T>& sob) AUG_NOTHROW
    {
        release<T>(static_cast<obref<T> >(sob));
    }

    inline bool
    equalid(const char* lhs, const char* rhs) AUG_NOTHROW
    {
        return AUG_EQUALID(lhs, rhs);
    }

    template <typename T>
    bool
    equalid(const char* id) AUG_NOTHROW
    {
        return equalid(object_traits<T>::id(), id);
    }

    template <typename T>
    smartob<T>
    object_attach(obref<T> ref) AUG_NOTHROW
    {
        return smartob<T>::attach(ref);
    }

    template <typename T>
    smartob<T>
    object_retain(obref<T> ref) AUG_NOTHROW
    {
        return smartob<T>::retain(ref);
    }

    template <typename T, typename U>
    smartob<T>
    object_cast(obref<U> ref) AUG_NOTHROW
    {
        return null == ref ? null : smartob<T>::attach
            (static_cast<T*>(ref.get()->vtbl_->cast_
                             (ref.get(), object_traits<T>::id())));
    }

    template <typename T, typename U>
    smartob<T>
    object_cast(const smartob<U>& sob) AUG_NOTHROW
    {
        return object_cast<T, U>(static_cast<obref<U> >(sob));
    }

    class ref_base {
        int refs_;
    protected:
        virtual
        ~ref_base() AUG_NOTHROW
        {
        }
        ref_base()
            : refs_(1)
        {
        }
    public:
        void
        retain_() AUG_NOTHROW
        {
            assert(0 < refs_);
            ++refs_;
        }
        void
        release_() AUG_NOTHROW
        {
            assert(0 < refs_);
            if (0 == --refs_)
                delete this;
        }
    };

    // Prototype for error verification.

    template <typename T, typename U>
    T
    verify(T result, obref<U> src);
}

template <typename T>
bool
isnull(aug::obref<T> ref)
{
    return 0 == ref.get();
}

template <typename T>
bool
isnull(const aug::smartob<T>& sobj)
{
    return 0 == sobj.get();
}

#endif // AUGABIPP_HPP
