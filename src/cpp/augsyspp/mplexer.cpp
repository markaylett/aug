/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYSPP_BUILD
#include "augsyspp/mplexer.hpp"

#include "augsys/string.h" // aug_perror()

#include <cerrno>

using namespace aug;
using namespace std;

AUGSYSPP_API
mplexer::~mplexer() NOTHROW
{
    if (-1 == aug_freemplexer(mplexer_))
        aug_perror("aug_freemplexer() failed");
}

AUGSYSPP_API int
aug::waitevents(aug_mplexer_t mplexer, const struct timeval& timeout)
{
    int ret(aug_waitevents(mplexer, &timeout));
    if (-1 == ret)
        error("aug_waitevents() failed");
    return ret;
}

AUGSYSPP_API int
aug::waitevents(aug_mplexer_t mplexer)
{
    int ret(aug_waitevents(mplexer, 0));
    if (-1 == ret)
        error("aug_waitevents() failed");
    return ret;
}

AUGSYSPP_API unsigned short
aug::eventmask(aug_mplexer_t mplexer, fdref ref)
{
    int ret(aug_eventmask(mplexer, ref.get()));
    if (-1 == ret)
        error("aug_eventmask() failed");

    return ret;
}

AUGSYSPP_API unsigned short
aug::events(aug_mplexer_t mplexer, fdref ref)
{
    int ret(aug_events(mplexer, ref.get()));
    if (-1 == ret)
        error("aug_events() failed");

    return ret;
}

AUGSYSPP_API pair<smartfd, smartfd>
aug::mplexerpipe()
{
    int fds[2];
    if (-1 == aug_mplexerpipe(fds))
        error("aug_mplexerpipe() failed");

    return make_pair(smartfd::attach(fds[0]), smartfd::attach(fds[1]));
}
