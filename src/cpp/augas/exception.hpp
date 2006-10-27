/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_EXCEPTION_HPP
#define AUGAS_EXCEPTION_HPP

#include "augsyspp/exception.hpp"

namespace augas {

    const int SRCMODULE(AUG_SRCUSER + 0);
    const int EMODCALL(1);

    typedef aug::basic_error<SRCMODULE> module_error;
}

#endif // AUGAS_EXCEPTION_HPP
