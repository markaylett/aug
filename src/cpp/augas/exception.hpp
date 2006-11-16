/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_EXCEPTION_HPP
#define AUGAS_EXCEPTION_HPP

#include "augsyspp/exception.hpp"

namespace augas {

    const int SRCAUGAS(AUG_SRCUSER + 0);
    const int ECONFIG(1);
    const int EMODCALL(2);
    const int ESTATE(3);

    typedef aug::basic_error<SRCAUGAS> error;
}

#endif // AUGAS_EXCEPTION_HPP
