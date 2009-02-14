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
#ifndef AUGMARPP_ITERATOR_HPP
#define AUGMARPP_ITERATOR_HPP

#include "augmarpp/mar.hpp"

#include "augctxpp/mpool.hpp"

#include <iterator>

namespace aug {

    class const_iterator : public mpool_ops {

        friend unsigned
        toord(const const_iterator&);

    public:
        typedef int difference_type;
        typedef const char* value_type;
        typedef value_type pointer;
        typedef value_type reference;
        typedef std::random_access_iterator_tag iterator_category;
        typedef unsigned size_type;
#if defined(_MSC_VER) && _MSC_VER < 1310
        typedef difference_type distance_type;
#endif // _MSC_VER < 1310
    private:

        marref ref_;
        difference_type ord_;

        void
        move(difference_type diff)
        {
            ord_ += diff;
        }
    public:
        const_iterator(marref ref, unsigned ord)
            : ref_(ref),
              ord_(static_cast<difference_type>(ord))
        {
        }
        reference
        operator *() const
        {
            const char* name;
            toname(ref_, name, ord_);
            return name;
        }
        const_iterator&
        operator ++()
        {
            move(1);
            return *this;
        }
        const const_iterator
        operator ++(int)
        {
            const_iterator tmp(*this);
            move(1);
            return tmp;
        }
        const_iterator&
        operator --()
        {
            move(-1);
            return *this;
        }
        const const_iterator
        operator --(int)
        {
            const_iterator tmp(*this);
            move(-1);
            return tmp;
        }
        const_iterator&
        operator +=(difference_type diff)
        {
            move(diff);
            return *this;
        }
        const_iterator&
        operator -=(difference_type diff)
        {
            move(-diff);
            return *this;
        }
        reference
        operator [](difference_type diff)
        {
            return *const_iterator(ref_, ord_ + diff);
        }
        bool
        operator ==(const const_iterator& rhs) const
        {
            return ref_ == rhs.ref_ && ord_ == rhs.ord_;
        }
        bool
        operator <(const const_iterator& rhs) const
        {
            return ref_.get() < ref_.get() && ord_ < rhs.ord_;
        }
    };

#if !defined(_MSC_VER) || _MSC_VER >= 1310
# if !defined(_RWSTD_VER)

    typedef std::reverse_iterator<
        const_iterator> const_reverse_iterator;

# else // _RWSTD_VER

    typedef std::reverse_iterator<
        const_iterator,
        std::random_access_iterator_tag,
        const_iterator::value_type,
        const_iterator::reference,
        const_iterator::pointer,
        const_iterator::difference_type> const_reverse_iterator;

# endif // _RWSTD_VER
#else // MSC_VER < 1310

    typedef std::reverse_iterator<
        const_iterator,
        const_iterator::value_type,
        const_iterator::reference,
        const_iterator::pointer,
        const_iterator::difference_type> const_reverse_iterator;

#endif // _MSC_VER < 1310

    inline unsigned
    toord(const const_iterator& it)
    {
        return it.ord_;
    }
    inline unsigned
    toord(const const_reverse_iterator& it)
    {
        return toord(it.base()) - 1;
    }
    inline const_iterator
    operator +(const const_iterator& lhs,
               const_iterator::difference_type diff)
    {
        const_iterator tmp(lhs);
        return tmp += diff;
    }
    inline const_iterator
    operator -(const const_iterator& lhs,
               const_iterator::difference_type diff)
    {
        const_iterator tmp(lhs);
        return tmp -= diff;
    }
    inline const_iterator::difference_type
    operator -(const const_iterator& lhs, const const_iterator& rhs)
    {
        return static_cast<const_iterator::difference_type>(toord(lhs)
                                                            - toord(rhs));
    }
    inline bool
    operator !=(const const_iterator& lhs, const const_iterator& rhs)
    {
        return !(lhs == rhs);
    }
    inline bool
    operator >(const const_iterator& lhs, const const_iterator& rhs)
    {
        return rhs < lhs;
    }
    inline bool
    operator <=(const const_iterator& lhs, const const_iterator& rhs)
    {
        return !(lhs > rhs);
    }
    inline bool
    operator >=(const const_iterator& lhs, const const_iterator& rhs)
    {
        return !(lhs < rhs);
    }
}

#endif // AUGMARPP_ITERATOR_HPP
