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
#ifndef AUGCTXPP_MPOOL_HPP
#define AUGCTXPP_MPOOL_HPP

#include "augctxpp/types.hpp"

#include "augctx/base.h"
#include "augctx/mpool.h"

#include <new>

namespace aug {

    inline mpoolptr
    getcrtmalloc()
    {
        return object_attach<aug_mpool>(aug_getcrtmalloc());
    }

    inline mpoolptr
    createdlmalloc()
    {
        return object_attach<aug_mpool>(aug_createdlmalloc());
    }

    class mpool_ops {
    protected:
        ~mpool_ops()
        {
        }
    public:

        // Aug overloads.

#if !defined(_MSC_VER)
        static void*
        operator new(std::size_t size, const tlx_&) throw(std::bad_alloc)
#else // _MSC_VER
        static void*
        operator new(std::size_t size, const tlx_&)
#endif // _MSC_VER
        {
            mpoolptr mpool(getmpool(aug_tlx));
            void* ptr = aug_allocmem(mpool.get(), size);
            if (!ptr)
                throw std::bad_alloc();
            return ptr;
        }

        static void
        operator delete(void* ptr, const tlx_&) throw()
        {
            mpoolptr mpool(getmpool(aug_tlx));
            aug_freemem(mpool.get(), ptr);
        }

        static void
        operator delete(void* ptr) throw()
        {
            mpoolptr mpool(getmpool(aug_tlx));
            aug_freemem(mpool.get(), ptr);
        }

        // Nothrow overloads.

        static void*
        operator new(std::size_t size, const std::nothrow_t&) throw()
        {
            return ::operator new(size, std::nothrow);
        }
        static void
        operator delete(void* ptr, const std::nothrow_t&) throw()
        {
            ::operator delete(ptr, std::nothrow);
        }

        // Placement overloads.

        static void*
        operator new(std::size_t size, void* place) throw()
        {
            return ::operator new(size, place);
        }
        static void
        operator delete(void* ptr, void* place) throw()
        {
            ::operator delete(ptr, place);
        }

        // Aug array overloads.

#if !defined(_MSC_VER)
        static void*
        operator new[](std::size_t size, const tlx_&) throw(std::bad_alloc)
#else // _MSC_VER
        static void*
        operator new[](std::size_t size, const tlx_&)
#endif // _MSC_VER
        {
            mpoolptr mpool(getmpool(aug_tlx));
            void* ptr = aug_allocmem(mpool.get(), size);
            if (!ptr)
                throw std::bad_alloc();
            return ptr;
        }

        static void
        operator delete[](void* ptr, const tlx_&) throw()
        {
            mpoolptr mpool(getmpool(aug_tlx));
            aug_freemem(mpool.get(), ptr);
        }

        static void
        operator delete[](void* ptr) throw()
        {
            mpoolptr mpool(getmpool(aug_tlx));
            aug_freemem(mpool.get(), ptr);
        }

        // Nothrow array overloads.

        static void*
        operator new[](std::size_t size, const std::nothrow_t&) throw()
        {
            return ::operator new[](size, std::nothrow);
        }
        static void
        operator delete[](void* ptr, const std::nothrow_t&) throw()
        {
            ::operator delete[](ptr, std::nothrow);
        }

        // Placement array overloads.

        static void*
        operator new[](std::size_t size, void* place) throw()
        {
            return ::operator new(size, place);
        }
        static void
        operator delete[](void* ptr, void* place) throw()
        {
            ::operator delete(ptr, place);
        }
    };
}

#endif // AUGCTXPP_MPOOL_HPP
