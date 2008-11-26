/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTXPP_MPOOL_HPP
#define AUGCTXPP_MPOOL_HPP

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

    const struct tlx_ { } tlx = tlx_();

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
