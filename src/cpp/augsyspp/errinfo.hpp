/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_ERRINFO_HPP
#define AUGSYSPP_ERRINFO_HPP

#include "augsyspp/config.hpp"

#include "augsys/errinfo.h"

namespace aug {

    inline const char*
    errfile(const struct aug_errinfo& errinfo)
    {
        return errinfo.file_;
    }

    inline int
    errline(const struct aug_errinfo& errinfo)
    {
        return errinfo.line_;
    }

    inline int
    errsrc(const struct aug_errinfo& errinfo)
    {
        return errinfo.src_;
    }

    inline int
    errnum(const struct aug_errinfo& errinfo)
    {
        return errinfo.num_;
    }

    inline const char*
    errdesc(const struct aug_errinfo& errinfo)
    {
        return errinfo.desc_;
    }
}

#endif // AUGSYSPP_ERRINFO_HPP
