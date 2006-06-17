/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNETPP_BUILD
#include "augnetpp/conn.hpp"

#include "augutil/list.h"

#include "augsys/errno.h"
#include "augsys/log.h"
#include "augsys/string.h" // aug_perror()

using namespace aug;

namespace {

    int
    poll_(void* arg, int id, struct aug_conns* conns)
    {
        try {
            poll_base* ptr = static_cast<poll_base*>(arg);
            return ptr->poll(id, *conns) ? 1 : 0;
        } AUG_CATCHRETURN 0; /* false */
    }
}

AUGNETPP_API
poll_base::~poll_base() NOTHROW
{
}

AUGNETPP_API
conns::~conns() NOTHROW
{
    if (-1 == aug_freeconns(&conns_))
        aug_perror("aug_freeconns() failed");
}

AUGNETPP_API
conns::conns()
{
    AUG_INIT(&conns_);
}

AUGNETPP_API bool
conns::empty() const
{
    return AUG_EMPTY(&conns_);
}

AUGNETPP_API void
aug::insertconn(struct aug_conns& conns, fdref ref, poll_base& action)
{
    if (-1 == aug_insertconn(&conns, ref.get(), poll_, &action))
        error("aug_insertconn() failed");
}
