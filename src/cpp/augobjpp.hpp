/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGOBJPP_HPP
#define AUGOBJPP_HPP

#include "augobj.h"

#include <algorithm> // swap()

#if !defined(AUG_NOTHROW)
# define AUG_NOTHROW throw()
#endif // AUG_NOTHROW

struct null_;

namespace aug {

    template <typename T>
    class smartobj;

    template <>
    class smartobj<aug_object> {

        aug_object* obj_;

        smartobj(aug_object* obj, bool retain) AUG_NOTHROW
            : obj_(obj)
        {
            if (obj && retain)
                aug_retainobject(obj);
        }

    public:
        ~smartobj() AUG_NOTHROW
        {
            if (obj_)
                aug_releaseobject(obj_);
        }

        smartobj(const null_&) AUG_NOTHROW
           : obj_(0)
        {
        }

        smartobj(const smartobj& rhs)
            : obj_(rhs.obj_)
        {
            if (obj_)
                aug_retainobject(obj_);
        }

        smartobj&
        operator =(const null_&) AUG_NOTHROW
        {
            *this = smartobj::retain(0);
            return *this;
        }

        smartobj&
        operator =(const smartobj& rhs)
        {
            smartobj tmp(rhs);
            swap(tmp);
            return *this;
        }

        void
        swap(smartobj& rhs) AUG_NOTHROW
        {
            std::swap(obj_, rhs.obj_);
        }

        void
        release()
        {
            if (obj_) {
                aug_object* obj(obj_);
                obj_ = 0;
                aug_releaseobject(obj);
            }
        }

        static smartobj
        attach(aug_object* obj)
        {
            return smartobj(obj, false);
        }

        static smartobj
        retain(aug_object* obj)
        {
            return smartobj(obj, true);
        }

        aug_object*
        get() const
        {
            return obj_;
        }

        operator aug_object*() const
        {
            return get();
        }
    };

    template <typename T>
    class smartobj {

        T* obj_;

        smartobj(T* obj, bool retain) AUG_NOTHROW
            : obj_(obj)
        {
            if (obj && retain)
                aug_retainobject(obj);
        }

    public:
        ~smartobj() AUG_NOTHROW
        {
            if (obj_)
                aug_releaseobject(obj_);
        }

        smartobj(const null_&) AUG_NOTHROW
           : obj_(0)
        {
        }

        smartobj(const smartobj& rhs)
            : obj_(rhs.obj_)
        {
            if (obj_)
                aug_retainobject(obj_);
        }

        smartobj&
        operator =(const null_&) AUG_NOTHROW
        {
            *this = smartobj::retain(0);
            return *this;
        }

        smartobj&
        operator =(const smartobj& rhs)
        {
            smartobj tmp(rhs);
            swap(tmp);
            return *this;
        }

        void
        swap(smartobj& rhs) AUG_NOTHROW
        {
            std::swap(obj_, rhs.obj_);
        }

        void
        release()
        {
            if (obj_) {
                T* obj(obj_);
                obj_ = 0;
                aug_releaseobject(obj);
            }
        }

        static smartobj
        attach(T* obj)
        {
            return smartobj(obj, false);
        }

        static smartobj
        retain(T* obj)
        {
            return smartobj(obj, true);
        }

        T*
        get() const
        {
            return obj_;
        }

        operator T*() const
        {
            return get();
        }

        operator aug_object*() const
        {
            return reinterpret_cast<aug_object*>(get());
        }
    };

    template <typename T>
    smartobj<T>
    castobject(T obj, const char* type)
    {
        return smartobj<T>::attach(obj->vtbl_->cast_(obj, type));
    }

    template <typename T>
    int
    retainobject(T obj)
    {
        return obj->vtbl_->retain_(obj);
    }

    template <typename T>
    int
    releaseobject(T obj)
    {
        return obj->vtbl_->release_(obj);
    }

    inline const void*
    blobdata(aug_blob* obj, size_t& size)
    {
        return obj->vtbl_->data_(obj, &size);
    }
}

template <typename T>
bool
isnull(const aug::smartobj<T>& sobj)
{
    return 0 == sobj.get();
}

#endif // AUGOBJPP_HPP
