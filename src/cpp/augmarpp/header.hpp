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
#ifndef AUGMARPP_HEADER_HPP
#define AUGMARPP_HEADER_HPP

#include "augmarpp/iterator.hpp"

#include "augctxpp/exception.hpp"

namespace aug {

    class header : public mpool_ops {
    public:
        typedef aug::const_iterator const_iterator;
        typedef aug::const_reverse_iterator const_reverse_iterator;
        typedef const_iterator::difference_type difference_type;
        typedef const_iterator::value_type value_type;
        typedef const_iterator::pointer const_pointer;
        typedef const_iterator::reference const_reference;
        typedef const_iterator::size_type size_type;
    private:
        marptr mar_;
    public:
        explicit
        header(marref ref)
            : mar_(object_retain(ref))
        {
        }
        void
        clearfields()
        {
            aug::clearfields(mar_);
        }
        const_iterator
        putfield(const aug_field& f)
        {
            return const_iterator(mar_, aug::putfield(mar_, f));
        }
        const_iterator
        putfield(const const_iterator& it, const void* cdata, unsigned n)
        {
            aug::putfieldn(mar_, distance(it), cdata, n);
            return it;
        }
        const_iterator
        putfield(const const_iterator& it, const char* cdata)
        {
            aug::putfieldn(mar_, distance(it), cdata,
                           static_cast<unsigned>(strlen(cdata)));
            return it;
        }
        const_iterator
        delfield(const const_iterator& it)
        {
            aug::delfieldn(mar_, distance(it));
            return it;
        }
        const void*
        getfield(const const_iterator& it) const
        {
            const void* value;
            aug::getfieldn(mar_, distance(it), value);
            return value;
        }
        const void*
        getfield(const const_iterator& it, unsigned& size) const
        {
            const void* value;
            size = aug::getfieldn(mar_, distance(it), value);
            return value;
        }
        const void*
        getfield(const const_reverse_iterator& it) const
        {
            const void* value;
            aug::getfieldn(mar_, distance(it), value);
            return value;
        }
        const void*
        getfield(const const_reverse_iterator& it, unsigned& size) const
        {
            const void* value;
            size = aug::getfieldn(mar_, distance(it), value);
            return value;
        }
        void
        getfield(const const_iterator& it, aug_field& f) const
        {
            aug::getfield(mar_, distance(it), f);
        }
        const_iterator
        find(const char* name) const
        {
            try {
                return const_iterator(mar_, fieldpton(mar_, name));
            } catch (const none_exception&) { }
            return end();
        }
        const_iterator
        begin() const
        {
            return const_iterator(mar_, 0);
        }
        const_iterator
        end() const
        {
            return const_iterator(mar_, size());
        }
        const_reverse_iterator
        rbegin() const
        {
            return const_reverse_iterator(end());
        }
        const_reverse_iterator
        rend() const
        {
            return const_reverse_iterator(begin());
        }
        bool
        empty() const
        {
            return 0 == size();
        }
        size_type
        size() const
        {
            return getfieldcount(mar_);
        }
    };
}

#endif // AUGMARPP_HEADER_HPP
