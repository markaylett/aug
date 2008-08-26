/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_MMAP_HPP
#define AUGSYSPP_MMAP_HPP

#include "augsyspp/types.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/utility.hpp" // perrinfo()

#include "augsys/mmap.h"

namespace aug {

    inline void
    remmap(aug_mmap& mm, size_t offset, size_t len)
    {
        verify(aug_remmap(&mm, offset, len));
    }

    inline void
    syncmmap(const aug_mmap& mm)
    {
        verify(aug_syncmmap(&mm));
    }

    inline size_t
    mmapsize(const aug_mmap& mm)
    {
        return aug_mmapsize(&mm);
    }

    class mmap {
    public:
        typedef aug_mmap ctype;
    private:
        aug_mmap* const mmap_;

        mmap(const mmap&);

        mmap&
        operator =(const mmap&);

    public:
        ~mmap() AUG_NOTHROW
        {
            if (-1 == aug_destroymmap(mmap_))
                perrinfo(aug_tlx, "aug_destroymmap() failed");
        }

        mmap(fdref ref, size_t offset, size_t len, int flags)
            : mmap_(aug_createmmap(ref.get(), offset, len, flags))
        {
            verify(mmap_);
        }

        void*
        addr() const
        {
            return mmap_->addr_;
        }

        size_t
        len() const
        {
            return mmap_->len_;
        }

        operator aug_mmap&()
        {
            return *mmap_;
        }

        operator const aug_mmap&() const
        {
            return *mmap_;
        }
    };
}

#endif // AUGSYSPP_MMAP_HPP
