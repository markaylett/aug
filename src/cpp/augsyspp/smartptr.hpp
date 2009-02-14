/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
#ifndef AUGSYSPP_SMARTPTR_HPP
#define AUGSYSPP_SMARTPTR_HPP

#include "augsyspp/config.hpp"

#include "augnullpp.hpp"

#include <algorithm> // swap()

/**
 * @file augsyspp/smartptr.hpp
 *
 * A simple shared pointer implementation.
 *
 * Where available, boost's shared_ptr<> should be preferred.
 */

namespace aug {

#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable:4101)
#endif // _MSC_VER

    namespace detail {
        struct cast_tag { };
    }

    template <typename T>
    class smartptr {

        template <typename U>
        friend class smartptr;

        T* ptr_;
        unsigned* refs_;

        void
        release()
        {
            T* ptr(0);
            unsigned* refs(0);

            // Swap instance state to locals.

            std::swap(ptr, ptr_);
            std::swap(refs, refs_);

            // Instance now null.

            if (!ptr || 0 < --*refs)
                return;

            delete ptr;
            delete refs;
        }
        void
        retain()
        {
            if (ptr_)
                ++*refs_;
        }

    public:
        ~smartptr() AUG_NOTHROW
        {
            release();
        }
        smartptr(const null_&) AUG_NOTHROW
        :   ptr_(0),
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
        :   ptr_(rhs.ptr_),
            refs_(rhs.refs_)
        {
            retain();
        }
        template <typename U>
        smartptr(const smartptr<U>& rhs) AUG_NOTHROW
        :   ptr_(rhs.ptr_),
            refs_(rhs.refs_)
        {
            retain();
        }
        template <typename U>
        smartptr(const smartptr<U>& rhs, const detail::cast_tag&) AUG_NOTHROW
        :   ptr_(dynamic_cast<T*>(rhs.ptr_)),
            refs_(ptr_ ? rhs.refs_ : 0)
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
            smartptr sptr(rhs);
            swap(sptr);
            return *this;
        }
        template <typename U>
        smartptr&
        operator =(const smartptr<U>& rhs) AUG_NOTHROW
        {
            smartptr sptr(rhs);
            swap(sptr);
            return *this;
        }
        void
        reset(T* ptr = 0)
        {
            smartptr sptr(ptr);
            swap(sptr);
        }
        void
        swap(smartptr& rhs) AUG_NOTHROW
        {
            std::swap(ptr_, rhs.ptr_);
            std::swap(refs_, rhs.refs_);
        }
        unsigned
        refs() const
        {
            return *refs_;
        }
        T*
        get() const AUG_NOTHROW
        {
            return ptr_;
        }
        T&
        operator *() const AUG_NOTHROW
        {
            return *ptr_;
        }
        T*
        operator ->() const AUG_NOTHROW
        {
            return ptr_;
        }
    };

    template <typename T, typename U>
    smartptr<T>
    smartptr_cast(const smartptr<U>& sptr)
    {
        return smartptr<T>(sptr, detail::cast_tag());
    }

#if defined(_MSC_VER)
# pragma warning(pop)
#endif // _MSC_VER
}

template <typename T>
bool
isnull(const aug::smartptr<T>& sptr)
{
    return 0 == sptr.get();
}

#endif // AUGSYSPP_SMARTPTR_HPP
