/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_SMARTPTR_HPP
#define AUGSYSPP_SMARTPTR_HPP

#include "augsyspp/lock.hpp"

#include <algorithm> // swap()

/**
   A simple shared pointer implementation.  Where available, boost's
   shared_ptr<> would be preferrable.
*/

namespace aug {

#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable:4101)
#endif // _MSC_VER

    template <typename T, typename U = null_>
    class smartptr {
        typedef U scoped_lock;
        T* ptr_;
        unsigned* refs_;

        void
        release()
        {
            T* ptr(0);
            unsigned* refs(0);
            {
                scoped_lock lock;
                if (!ptr_ || 0 < --*refs_)
                    return;

                std::swap(ptr_, ptr);
                std::swap(refs_, refs);
            }

            /**
               Release lock before deleting: ~T() may use aug_lock().
            */

            delete ptr;
            delete refs;
        }
        void
        retain()
        {
            scoped_lock lock;
            if (ptr_)
                ++*refs_;
        }

    public:
        ~smartptr() AUG_NOTHROW
        {
            release();
        }
        smartptr(const null_&) AUG_NOTHROW
        : ptr_(0),
            refs_(0)
        {
        }
        explicit
        smartptr(T* ptr = 0)
            : ptr_(ptr),
              refs_(ptr ? new unsigned(1) : 0)
        {
        }
        smartptr(const smartptr& rhs) AUG_NOTHROW
        : ptr_(rhs.ptr_),
            refs_(rhs.refs_)
        {
            retain();
        }
        smartptr&
        operator =(const null_&) AUG_NOTHROW
        {
            release();
            return *this;
        }
        smartptr&
        operator =(const smartptr& rhs) AUG_NOTHROW
        {
            smartptr ptr(rhs);
            swap(ptr);
            return *this;
        }
        void
        swap(smartptr& rhs) AUG_NOTHROW
        {
            scoped_lock lock;
            std::swap(ptr_, rhs.ptr_);
            std::swap(refs_, rhs.refs_);
        }
        unsigned
        refs() const
        {
            scoped_lock lock;
            return *refs_;
        }
        T*
        get() const AUG_NOTHROW
        {
            scoped_lock lock;
            return ptr_;
        }
        T&
        operator*() const AUG_NOTHROW
        {
            scoped_lock lock;
            return *ptr_;
        }
        T*
        operator->() const AUG_NOTHROW
        {
            scoped_lock lock;
            return ptr_;
        }
    };

#if defined(_MSC_VER)
# pragma warning(pop)
#endif // _MSC_VER
}

template <typename T, typename U>
bool
isnull(const aug::smartptr<T, U>& ptr)
{
    return 0 == ptr.get();
}

#endif // AUGSYSPP_SMARTPTR_HPP
