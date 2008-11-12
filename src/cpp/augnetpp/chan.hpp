/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_CHAN_HPP
#define AUGNETPP_CHAN_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/types.hpp" // sdref

#include "augctxpp/exception.hpp"

#include "augnet/chan.h"

#include "augsys/muxer.h"

#include "augext/chan.h"
#include "augext/mpool.h"

namespace aug {

    inline chanptr
    createclient(mpoolref mpool, const char* host, const char* serv,
                 aug_muxer_t muxer, struct ssl_ctx_st* sslctx = 0)
    {
        return object_attach<aug_chan>
            (verify(aug_createclient(mpool.get(), host, serv, muxer,
                                     sslctx)));
    }

    inline chanptr
    createserver(mpoolref mpool, aug_muxer_t muxer, sdref sd,
                 struct ssl_ctx_st* sslctx = 0)
    {
        return object_attach<aug_chan>
            (verify(aug_createserver(mpool.get(), muxer, sd.get(), sslctx)));
    }

    inline chanptr
    createplain(mpoolref mpool, unsigned id, aug_muxer_t muxer, sdref sd,
                unsigned short mask)
    {
        return object_attach<aug_chan>
            (verify(aug_createplain(mpool.get(), id, muxer, sd.get(), mask)));
    }
}

#endif // AUGNETPP_CHAN_HPP
