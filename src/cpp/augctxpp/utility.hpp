/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
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
             const struct aug_errinfo& errinfo) AUG_NOTHROW
    {
        return aug_perrinfo(ctx.get(), s, &errinfo);
    }
}

#endif // AUGCTXPP_UTILITY_HPP
