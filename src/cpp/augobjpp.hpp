/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGOBJPP_HPP
#define AUGOBJPP_HPP

#include "augobj.h"

#include <algorithm> // swap()

#if !defined(AUG_NOTHROW)
# define AUG_NOTHROW
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

namespace aug {

    template <typename T>
    class obref;

    template <>
    class obref<aug_object> {
    public:
        typedef aug_object obtype;
    protected:
        void* ptr_;
        obref(void* ptr)
            : ptr_(ptr)
        {
        }
    public:
        obref(const null_&)
            : ptr_(0)
        {
        }
        obref(aug_object* ptr)
            : ptr_(ptr)
        {
        }
        aug_object*
        get() const
        {
            return static_cast<aug_object*>(ptr_);
        }
    };

    typedef obref<aug_object> objectref;

    template <typename T>
    class obref : public objectref {
        typedef T obtype;
    public:
        obref(const null_&)
            : objectref(null)
        {
        }
        obref(T* ptr)
            : objectref(ptr)
        {
        }
        T*
        get() const
        {
            return static_cast<T*>(ptr_);
        }
    };

    template <typename T>
    obref<T>
    makeref(T* ptr)
    {
        return obref<T>(ptr);
    }

    template <typename T>
    int
    incref(obref<T> ref)
    {
        return ref.get()->vtbl_->incref_(ref.get());
    }

    template <typename T>
    T*
    incget(const obref<T>& ref)
    {
        incref(ref);
        return ref.get();
    }

    template <typename T>
    int
    decref(obref<T> ref)
    {
        return ref.get()->vtbl_->decref_(ref.get());
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
            if (ref.get() && inc)
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

        smartob(const smartob& rhs)
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
        operator =(const smartob& rhs)
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
        attach(obref<T> ref)
        {
            return smartob(ref, false);
        }

        static smartob
        incref(obref<T> ref)
        {
            return smartob(ref, true);
        }

        T*
        get() const
        {
            return ref_.get();
        }

        operator obref<T>() const
        {
            return ref_;
        }
    };

    template <typename T>
    struct object_traits;

    template <typename T>
    int
    incref(const smartob<T>& sob)
    {
        return incref<T>(static_cast<obref<T> >(sob));
    }

    template <typename T>
    T*
    incget(const smartob<T>& sob)
    {
        incref(sob);
        return sob.get();
    }

    template <typename T>
    int
    decref(const smartob<T>& sob)
    {
        return decref<T>(static_cast<obref<T> >(sob));
    }

    inline bool
    equalid(const char* lhs, const char* rhs)
    {
        return AUG_EQUALID(lhs, rhs);
    }

    template <typename T>
    bool
    equalid(const char* id)
    {
        return equalid(object_traits<T>::id(), id);
    }

    template <>
    struct object_traits<aug_object> {
        typedef aug_objectvtbl vtbl;
        static const char*
        id()
        {
            return aug_objectid;
        }
    };

    template <typename T>
    smartob<T>
    object_attach(obref<T> ref)
    {
        return smartob<T>::attach(ref);
    }

    template <typename T, typename U>
    smartob<T>
    object_cast(obref<U> ref)
    {
        return smartob<T>::attach
            (static_cast<T*>(ref.get()->vtbl_->cast_
                             (ref.get(), object_traits<T>::id())));
    }

    template <typename T, typename U>
    smartob<T>
    object_cast(const smartob<U>& sob)
    {
        return object_cast<T, U>(static_cast<obref<U> >(sob));
    }

    class ref_base {
        unsigned refs_;
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
        incref_()
        {
            ++refs_;
            return 0;
        }
        int
        decref_()
        {
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
