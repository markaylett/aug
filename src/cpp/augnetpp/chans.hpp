/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_CHANS_HPP
#define AUGNETPP_CHANS_HPP

#include "augutilpp/object.hpp"

#include "augsyspp/exception.hpp"

#include "augnet/chans.h"

namespace aug {

    template <bool (*T)(objectref, unsigned, streamref, unsigned short)>
    aug_bool
    chancb(aug_object* ob, unsigned id, aug_stream* stream,
           unsigned short events) AUG_NOTHROW
    {
        try {
            return T(ob, id, stream, events) ? AUG_TRUE : AUG_FALSE;
        } AUG_SETERRINFOCATCH;

        /**
         * Do not remove the channel unless explicitly asked to.
         */

        return AUG_TRUE;
    }

    template <typename T, bool (T::*U)(unsigned, streamref, unsigned short)>
    aug_bool
    chanmemcb(aug_object* ob, unsigned id, aug_stream* stream,
              unsigned short events) AUG_NOTHROW
    {
        try {
            return (obtop<T*>(ob)->*U)(id, stream, events)
                ? AUG_TRUE : AUG_FALSE;
        } AUG_SETERRINFOCATCH;

        /**
         * Do not remove the channel unless explicitly asked to.
         */

        return AUG_TRUE;
    }

    template <typename T>
    aug_bool
    chanmemcb(aug_object* ob, unsigned id, aug_stream* stream,
              unsigned short events) AUG_NOTHROW
    {
        try {
            return obtop<T*>(ob)->chancb(id, stream, events)
                ? AUG_TRUE : AUG_FALSE;
        } AUG_SETERRINFOCATCH;

        /**
         * Do not remove the channel unless explicitly asked to.
         */

        return AUG_TRUE;
    }

    class chans {

        aug_chans_t chans_;

        chans(const chans&);

        chans&
        operator =(const chans&);

    public:
        ~chans() AUG_NOTHROW
        {
            aug_destroychans(chans_);
        }

        explicit
        chans(mpoolref mpool)
            : chans_(aug_createchans(mpool.get()))
        {
            verify(chans_);
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
    insertchan(aug_chans_t chans, chanref chan)
    {
        verify(aug_insertchan(chans, chan.get()));
    }

    inline bool
    removechan(aug_chans_t chans, unsigned id)
    {
        return AUG_FAILNONE == verify(aug_removechan(chans, id))
            ? false : true;
    }

    inline void
    foreachchan(aug_chans_t chans, aug_chancb_t cb, objectref ob)
    {
        aug_foreachchan(chans, cb, ob.get());
    }

    inline void
    foreachchan(aug_chans_t chans, aug_chancb_t cb, const null_&)
    {
        aug_foreachchan(chans, cb, 0);
    }

    template <typename T, bool (T::*U)(unsigned, streamref, unsigned short)>
    void
    foreachchan(aug_chans_t chans, T& x)
    {
        scoped_boxptr<simple_boxptr> ob(&x);
        aug_foreachchan(chans, chanmemcb<T, U>, ob.base());
    }

    template <typename T>
    void
    foreachchan(aug_chans_t chans, T& x)
    {
        scoped_boxptr<simple_boxptr> ob(&x);
        aug_foreachchan(chans, chanmemcb<T>, ob.base());
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
}

#endif // AUGNETPP_CHANS_HPP
