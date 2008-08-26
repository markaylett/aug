/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_BASE_HPP
#define AUGSYSPP_BASE_HPP

#include "augsys/base.h"

namespace aug {

    inline unsigned
    nextid()
    {
        return aug_nextid();
    }
}

#endif // AUGSYSPP_BASE_HPP
