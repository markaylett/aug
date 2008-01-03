/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUBPP_HPP
#define AUBPP_HPP

#include "aub.h"
#include "null.hpp"

#include <algorithm> // swap()
#include <cassert>

#if !defined(AUB_NOTHROW)
# if !defined(NDEBUG)
#  define AUB_NOTHROW throw()
# else /* NDEBUG */
#  define AUB_NOTHROW
# endif /* NDEBUG */
#endif /* !AUB_NOTHROW */

#define AUB_OBJECTASSERT(T) \
        do { (void)sizeof(aub::object_traits<T>::vtbl); } while (0)

namespace aub {

    template <typename T>
    struct object_traits;

    template <>
    struct object_traits<aub_object> {
        typedef aub_objectvtbl vtbl;
        static const char*
        id() AUB_NOTHROW
        {
            return aub_objectid;
        }
    };

    template <typename T>
    class obref;

    /**
     * Specialization for base object.
     */

    template <>
    class obref<aub_object> {
    public:
        typedef aub_object obtype;
        typedef object_traits<aub_object>::vtbl vtbl;
    protected:
        void* ptr_;
        obref(void* ptr) AUB_NOTHROW
            : ptr_(ptr)
        {
        }
    public:
        obref(const null_&) AUB_NOTHROW
            : ptr_(0)
        {
        }
        obref(aub_object* ptr) AUB_NOTHROW
            : ptr_(ptr)
        {
        }
        aub_object*
        get() const AUB_NOTHROW
        {
            return static_cast<aub_object*>(ptr_);
        }
    };

    typedef obref<aub_object> objectref;

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
        obref(const null_&) AUB_NOTHROW
            : objectref(null)
        {
        }
        obref(T* ptr) AUB_NOTHROW
            : objectref(ptr)
        {
        }
        T*
        get() const AUB_NOTHROW
        {
            return static_cast<T*>(ptr_);
        }
    };

    /**
     * Argument must not be null.
     */

    template <typename T>
    void
    retain(obref<T> ref) AUB_NOTHROW
    {
        assert(ref.get());
        ref.get()->vtbl_->retain_(ref.get());
    }

    /**
     * Argument must not be null.
     */

    template <typename T>
    T*
    incget(const obref<T>& ref) AUB_NOTHROW
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
    release(obref<T> ref) AUB_NOTHROW
    {
        assert(ref.get());
        ref.get()->vtbl_->release_(ref.get());
    }

    template <typename T>
    obref<T>
    obptr(T* ptr) AUB_NOTHROW
    {
        return obref<T>(ptr);
    }

    template <typename T>
    class smartob {
    public:
        typedef T obtype;
    private:
        obref<T> ref_;

        smartob(obref<T> ref, bool inc) AUB_NOTHROW
            : ref_(ref)
        {
            if (null != ref && inc)
                aub::retain(ref);
        }

    public:
        ~smartob() AUB_NOTHROW
        {
            if (null != ref_)
                aub::release(ref_);
        }
        smartob(const null_&) AUB_NOTHROW
            : ref_(null)
        {
        }
        smartob(const smartob& rhs) AUB_NOTHROW
            : ref_(rhs.ref_)
        {
            if (null != ref_)
                aub::retain(ref_);
        }
        smartob&
        operator =(const null_&) AUB_NOTHROW
        {
            *this = smartob(null);
            return *this;
        }
        smartob&
        operator =(const smartob& rhs) AUB_NOTHROW
        {
            smartob tmp(rhs);
            swap(tmp);
            return *this;
        }
        void
        swap(smartob& rhs) AUB_NOTHROW
        {
            std::swap(ref_, rhs.ref_);
        }
        static smartob
        attach(obref<T> ref) AUB_NOTHROW
        {
            return smartob(ref, false);
        }
        static smartob
        retain(obref<T> ref) AUB_NOTHROW
        {
            return smartob(ref, true);
        }
        aub_object*
        base() const AUB_NOTHROW
        {
            return obref<aub_object>(ref_).get();
        }
        T*
        get() const AUB_NOTHROW
        {
            return ref_.get();
        }
        operator obref<T>() const AUB_NOTHROW
        {
            return ref_;
        }
    };

    /**
     * Argument may be null.
     */

    template <typename T>
    void
    retain(const smartob<T>& sob) AUB_NOTHROW
    {
        retain<T>(static_cast<obref<T> >(sob));
    }

    template <typename T>
    T*
    incget(const smartob<T>& sob) AUB_NOTHROW
    {
        return incget<T>(static_cast<obref<T> >(sob));
    }

    template <typename T>
    void
    release(const smartob<T>& sob) AUB_NOTHROW
    {
        release<T>(static_cast<obref<T> >(sob));
    }

    inline bool
    equalid(const char* lhs, const char* rhs) AUB_NOTHROW
    {
        return AUB_EQUALID(lhs, rhs);
    }

    template <typename T>
    bool
    equalid(const char* id) AUB_NOTHROW
    {
        return equalid(object_traits<T>::id(), id);
    }

    template <typename T>
    smartob<T>
    object_attach(obref<T> ref) AUB_NOTHROW
    {
        return smartob<T>::attach(ref);
    }

    template <typename T>
    smartob<T>
    object_retain(obref<T> ref) AUB_NOTHROW
    {
        return smartob<T>::retain(ref);
    }

    template <typename T, typename U>
    smartob<T>
    object_cast(obref<U> ref) AUB_NOTHROW
    {
        return null == ref ? null : smartob<T>::attach
            (static_cast<T*>(ref.get()->vtbl_->cast_
                             (ref.get(), object_traits<T>::id())));
    }

    template <typename T, typename U>
    smartob<T>
    object_cast(const smartob<U>& sob) AUB_NOTHROW
    {
        return object_cast<T, U>(static_cast<obref<U> >(sob));
    }

    class ref_base {
        int refs_;
    protected:
        virtual
        ~ref_base() AUB_NOTHROW
        {
        }
        ref_base()
            : refs_(1)
        {
        }
    public:
        void
        retain_() AUB_NOTHROW
        {
            assert(0 < refs_);
            ++refs_;
        }
        void
        release_() AUB_NOTHROW
        {
            assert(0 < refs_);
            if (0 == --refs_)
                delete this;
        }
    };
}

template <typename T>
bool
isnull(aub::obref<T> ref)
{
    return 0 == ref.get();
}

template <typename T>
bool
isnull(const aub::smartob<T>& sobj)
{
    return 0 == sobj.get();
}

#endif // AUBPP_HPP
