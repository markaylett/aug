/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_CHANS_HPP
#define AUGNETPP_CHANS_HPP

#include "augctxpp/exception.hpp"

#include "augnet/chans.h"

namespace aug {

    class chans {

        aug_chans_t chans_;

        chans(const chans&);

        chans&
        operator =(const chans&);

    public:
        ~chans() AUG_NOTHROW
        {
            if (chans_)
                aug_destroychans(chans_);
        }

        chans(const null_&) AUG_NOTHROW
           : chans_(0)
        {
        }

        chans(mpoolref mpool, chandlerref chandler)
            : chans_(aug_createchans(mpool.get(), chandler.get()))
        {
            verify(chans_);
        }

        void
        swap(chans& rhs) AUG_NOTHROW
        {
            std::swap(chans_, rhs.chans_);
        }

        operator aug_chans_t()
        {
            return chans_;
        }

        aug_chans_t
        get()
        {
            return chans_;
        }
    };

    inline void
    swap(chans& lhs, chans& rhs) AUG_NOTHROW
    {
        lhs.swap(rhs);
    }

    inline void
    insertchan(aug_chans_t chans, chanref chan)
    {
        verify(aug_insertchan(chans, chan.get()));
    }

    inline void
    removechan(aug_chans_t chans, unsigned id)
    {
        verify(aug_removechan(chans, id));
    }

    inline chanptr
    findchan(aug_chans_t chans, unsigned id)
    {
        return object_attach<aug_chan>(aug_findchan(chans, id));
    }

    inline void
    processchans(aug_chans_t chans)
    {
        aug_processchans(chans);
    }

    inline void
    dumpchans(aug_chans_t chans)
    {
        aug_dumpchans(chans);
    }

    inline unsigned
    getchans(aug_chans_t chans)
    {
        return aug_getchans(chans);
    }

    inline unsigned
    getreadychans(aug_chans_t chans)
    {
        return aug_getreadychans(chans);
    }
}

#endif // AUGNETPP_CHANS_HPP
