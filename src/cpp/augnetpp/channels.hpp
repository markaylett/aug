/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_CHANNELS_HPP
#define AUGNETPP_CHANNELS_HPP

#include "augsyspp/exception.hpp"

#include "augnet/channels.h"

namespace aug {

    template <bool (*T)(unsigned, aug::streamobref, unsigned short)>
    aug_bool
    channelcb(unsigned id, aug_streamob* streamob,
              unsigned short events) AUG_NOTHROW
    {
        try {
            return T(id, streamob, events) ? AUG_TRUE : AUG_FALSE;
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
        channels(mpoolref mpool)
            : channels_(aug_createchannels(mpool.get()))
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
    insertchannel(aug_channels_t channels, channelobref channelob)
    {
        verify(aug_insertchannel(channels, channelob.get()));
    }

    inline bool
    removechannel(aug_channels_t channels, unsigned id)
    {
        return AUG_FAILNONE == verify(aug_removechannel(channels, id))
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

#endif // AUGNETPP_CHANNELS_HPP
