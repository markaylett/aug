/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTXPP_ERRINFO_HPP
#define AUGCTXPP_ERRINFO_HPP

#include "augctxpp/config.hpp"

#include "augctx/errinfo.h"

namespace aug {

    inline const char*
    errfile(const aug_errinfo& errinfo)
    {
        return errinfo.file_;
    }

    inline int
    errline(const aug_errinfo& errinfo)
    {
        return errinfo.line_;
    }

    inline const char*
    errsrc(const aug_errinfo& errinfo)
    {
        return errinfo.src_;
    }

    inline int
    errnum(const aug_errinfo& errinfo)
    {
        return errinfo.num_;
    }

    inline const char*
    errdesc(const aug_errinfo& errinfo)
    {
        return errinfo.desc_;
    }
}

#endif // AUGCTXPP_ERRINFO_HPP
