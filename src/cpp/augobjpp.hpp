/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGOBJPP_HPP
#define AUGOBJPP_HPP

#include "augobj.h"

#include <algorithm> // swap()
#include <cassert>

#if !defined(AUG_NOTHROW)
# if !defined(NDEBUG)
#  define AUG_NOTHROW throw()
# else /* NDEBUG */
#  define AUG_NOTHROW
# endif /* NDEBUG */
#endif /* !AUG_NOTHROW */

#if !defined(AUG_NULL)
# define AUG_NULL
const struct null_ { } null = null_();

template <typename typeT>
inline bool
operator ==(const typeT& lhs, const null_&)
{
    return isnull(lhs);
}

template <typename typeT>
inline bool
operator ==(const null_&, const typeT& rhs)
{
    return isnull(rhs);
}

template <typename typeT>
inline bool
operator !=(const typeT& lhs, const null_&)
{
    return !isnull(lhs);
}

template <typename typeT>
inline bool
operator !=(const null_&, const typeT& rhs)
{
    return !isnull(rhs);
}
#endif // AUG_NULL

#define AUG_OBJECTASSERT(T) \
        do { (void)sizeof(aug::object_traits<T>::vtbl); } while (0)

namespace aug {

    template <typename T>
    struct object_traits;

    template <typename T>
    class obref;

    template <>
    class obref<aug_object> {
    public:
        typedef aug_object obtype;
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
        get() const AUG_NOTHROW
        {
            return static_cast<aug_object*>(ptr_);
        }
    };

    typedef obref<aug_object> objectref;

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

    template <typename T>
    int
    incref(obref<T> ref) AUG_NOTHROW
    {
        assert(ref.get());
        return ref.get()->vtbl_->incref_(ref.get());
    }

    template <typename T>
    T*
    incget(const obref<T>& ref) AUG_NOTHROW
    {
        if (ref != null)
            incref(ref);
        return ref.get();
    }

    template <typename T>
    int
    decref(obref<T> ref) AUG_NOTHROW
    {
        assert(ref.get());
        return ref.get()->vtbl_->decref_(ref.get());
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

        smartob(obref<T> ref, bool inc) AUG_NOTHROW
            : ref_(ref)
        {
            if (null != ref && inc)
                aug::incref(ref);
        }

    public:
        ~smartob() AUG_NOTHROW
        {
            if (null != ref_)
                aug::decref(ref_);
        }

        smartob(const null_&) AUG_NOTHROW
            : ref_(null)
        {
        }

        smartob(const smartob& rhs) AUG_NOTHROW
            : ref_(rhs.ref_)
        {
            if (null != ref_)
                aug::incref(ref_);
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
        incref(obref<T> ref) AUG_NOTHROW
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

        operator obref<T>() const AUG_NOTHROW
        {
            return ref_;
        }
    };

    template <typename T>
    int
    incref(const smartob<T>& sob) AUG_NOTHROW
    {
        return incref<T>(static_cast<obref<T> >(sob));
    }

    template <typename T>
    T*
    incget(const smartob<T>& sob) AUG_NOTHROW
    {
        return incget<T>(static_cast<obref<T> >(sob));
    }

    template <typename T>
    int
    decref(const smartob<T>& sob) AUG_NOTHROW
    {
        return decref<T>(static_cast<obref<T> >(sob));
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
    smartob<T>
    object_attach(obref<T> ref) AUG_NOTHROW
    {
        return smartob<T>::attach(ref);
    }

    template <typename T>
    smartob<T>
    object_incref(obref<T> ref) AUG_NOTHROW
    {
        return smartob<T>::incref(ref);
    }

    template <typename T, typename U>
    smartob<T>
    object_cast(obref<U> ref) AUG_NOTHROW
    {
        return smartob<T>::attach
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
        int
        incref_() AUG_NOTHROW
        {
            assert(0 < refs_);
            ++refs_;
            return 0;
        }
        int
        decref_() AUG_NOTHROW
        {
            assert(0 < refs_);
            if (0 == --refs_)
                delete this;
            return 0;
        }
    };
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

#endif // AUGOBJPP_HPP
