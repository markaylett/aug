/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
