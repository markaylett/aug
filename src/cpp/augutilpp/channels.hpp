/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_CHANNEL_HPP
#define AUGUTILPP_CHANNEL_HPP

#include "augsyspp/exception.hpp"

#include "augutil/channels.h"

namespace aug {

    template <bool (*T)(aug::channelobref)>
    aug_bool
    channelcb(aug_channelob* ob) AUG_NOTHROW
    {
        try {
            return T(ob) ? AUG_TRUE : AUG_FALSE;
        } AUG_SETERRINFOCATCH;

        /**
         * Do not remove the channel unless explicitly asked to.
         */

        return AUG_TRUE;
    }

    class channels {

        aug_channels_t channels_;

        channels(const channels&);

        channels&
        operator =(const channels&);

    public:
        ~channels() AUG_NOTHROW
        {
            aug_destroychannels(channels_);
        }

        explicit
        channels(aug_mpool* mpool)
            : channels_(aug_createchannels(mpool))
        {
            verify(channels_);
        }

        operator aug_channels_t()
        {
            return channels_;
        }

        aug_channels_t
        get()
        {
            return channels_;
        }
    };

    inline void
    insertchannel(aug_channels_t channels, aug::obref<aug_channelob> ob)
    {
        verify(aug_insertchannel(channels, ob.get()));
    }

    inline bool
    removechannel(aug_channels_t channels, aug::obref<aug_channelob> ob)
    {
        return AUG_FAILNONE == verify(aug_removechannel(channels, ob.get()))
            ? false : true;
    }

    inline void
    foreachchannel(aug_channels_t channels, aug_channelcb_t cb)
    {
        aug_foreachchannel(channels, cb);
    }

    inline unsigned
    getchannels(aug_channels_t channels)
    {
        return aug_getchannels(channels);
    }
}

#endif // AUGUTILPP_CHANNEL_HPP
