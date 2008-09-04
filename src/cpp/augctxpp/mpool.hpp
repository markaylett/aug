/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTXPP_MPOOL_HPP
#define AUGCTXPP_MPOOL_HPP

#include "augctx/base.h"

#include <new>

namespace aug {

    class mpool_base {
    protected:
        ~mpool_base()
        {
        }
    public:

        // Normal overloads.

#if !defined(_MSC_VER)
        static void*
        operator new(std::size_t size) throw(std::bad_alloc)
#else // _MSC_VER
        static void*
        operator new(std::size_t size)
#endif // _MSC_VER
        {
            mpoolptr mpool(getmpool(aug_tlx));
            void* ptr = aug_allocmem(mpool.get(), size);
            if (!ptr)
                throw std::bad_alloc();
            return ptr;
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
            mpoolptr mpool(getmpool(aug_tlx));
            return aug_allocmem(mpool.get(), size);
        }
        static void
        operator delete(void* ptr, const std::nothrow_t&) throw()
        {
            mpoolptr mpool(getmpool(aug_tlx));
            aug_freemem(mpool.get(), ptr);
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

        // Normal array overloads.

#if !defined(_MSC_VER)
        static void*
        operator new[](std::size_t size) throw(std::bad_alloc)
#else // _MSC_VER
        static void*
        operator new[](std::size_t size)
#endif // _MSC_VER
        {
            mpoolptr mpool(getmpool(aug_tlx));
            void* ptr = aug_allocmem(mpool.get(), size);
            if (!ptr)
                throw std::bad_alloc();
            return ptr;
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
            mpoolptr mpool(getmpool(aug_tlx));
            return aug_allocmem(mpool.get(), size);
        }
        static void
        operator delete[](void* ptr, const std::nothrow_t&) throw()
        {
            mpoolptr mpool(getmpool(aug_tlx));
            aug_freemem(mpool.get(), ptr);
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
