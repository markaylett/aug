/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

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
#ifndef AUGCTXPP_UTILITY_HPP
#define AUGCTXPP_UTILITY_HPP

#include "augctx/ctx.h" // aug_perrinfo()

namespace aug {

    /**
     * Print last error.
     *
     * Akin to perror().  Can be safely called from destructors and catch
     * blocks.
     *
     * @param s String to be prepended.
     *
     * @return -1 on error.
     */

    inline aug_result
    perrinfo(ctxref ctx, const char* s) AUG_NOTHROW
    {
        return aug_perrinfo(ctx.get(), s, NULL);
    }

    /**
     * Print last error.
     *
     * @param ctx Context.
     *
     * @param s String to be prepended.
     *
     * @return -1 on error.
     */

    inline aug_result
    perrinfo(ctxref ctx, const char* s,
             const aug_errinfo& errinfo) AUG_NOTHROW
    {
        return aug_perrinfo(ctx.get(), s, &errinfo);
    }
}

#endif // AUGCTXPP_UTILITY_HPP
