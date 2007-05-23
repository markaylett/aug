/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/engine.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augrtpp/conn.hpp"
#include "augrtpp/servs.hpp"
#include "augrtpp/socks.hpp"
#include "augrtpp/ssl.hpp"

#include "augnetpp/nbfile.hpp"

#include "augutilpp/timer.hpp"

#include <map>
#include <queue>

using namespace aug;
using namespace std;

namespace {
    typedef map<int, servptr> idtoserv;
    typedef queue<connptr> pending;
}

namespace aug {
    namespace detail {
        struct engineimpl {

            nbfiles nbfiles_;
            servs servs_;
            socks socks_;
            timers timers_;
            timer grace_;

            // Mapping of timer-ids to services.

            idtoserv idtoserv_;

            // Pending calls to connected().

            pending pending_;

            enum {
                STARTED,
                TEARDOWN,
                STOPPED
            } state_;

            engineimpl()
                : grace_(timers_),
                  state_(STARTED)
            {
            }
        };
    }
}

engine::~engine() AUG_NOTHROW
{
    delete impl_;
}

engine::engine()
    : impl_(new detail::engineimpl())
{
}

bool
engine::stop() const
{
    return detail::engineimpl::STARTED != impl_->state_;
}
