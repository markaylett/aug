/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_EXCEPTION_HPP
#define DAUG_EXCEPTION_HPP

#include "augsyspp/exception.hpp"

namespace augas {

    const int SRCDAUG(AUG_SRCUSER + 0);
    const int ECONFIG(1);
    const int EHOSTCALL(2);
    const int EMODCALL(3);
    const int ESSLCTX(4);
    const int ESTATE(5);

    typedef aug::basic_error<SRCDAUG> error;
}

#endif // DAUG_EXCEPTION_HPP
