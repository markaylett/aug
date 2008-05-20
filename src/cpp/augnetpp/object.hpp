/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_OBJECT_HPP
#define AUGNETPP_OBJECT_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/exception.hpp"

#include "augnet/object.h"

#include "augsys/muxer.h"

#include "augob/channelob.h"

#include "augctx/mpool.h"

namespace aug {

    inline channelobptr
    createclient(mpoolref mpool, const char* host, const char* serv,
                 aug_muxer_t muxer, struct ssl_st* ssl)
    {
        return object_attach<aug_channelob>
            (verify(aug_createclient(mpool.get(), host, serv, muxer, ssl)));
    }

    inline channelobptr
    aug_createserver(mpoolref mpool, sdref sd, aug_muxer_t muxer,
                     struct ssl_st* ssl)
    {
        return object_attach<aug_channelob>
            (verify(aug_createserver(mpool.get(), sd.get(), muxer, ssl)));
    }

    inline channelobptr
    createplain(mpoolref mpool, unsigned id, sdref sd, aug_muxer_t muxer)
    {
        return object_attach<aug_channelob>
            (verify(aug_createplain(mpool.get(), id, sd.get(), muxer)));
    }
}

#endif // AUGNETPP_OBJECT_HPP
