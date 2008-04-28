/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_UTILITY_HPP
#define AUGSYSPP_UTILITY_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augsys/base.h"   // aug_getosfd()
#include "augsys/unistd.h" // aug_fsize()
#include "augsys/utility.h"

namespace aug {

    /**
     * Get size of file in bytes.
     *
     * @param ref File descriptor.
     *
     * @return Size in bytes.
     */

    inline size_t
    fsize(fdref ref)
    {
        size_t size;
        verify(aug_fsize(ref.get(), &size));
        return size;
    }

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
    perrinfo(aug_ctx* ctx, const char* s) AUG_NOTHROW
    {
        return aug_perrinfo(ctx, s, NULL);
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
    perrinfo(aug_ctx* ctx, const char* s,
             const struct aug_errinfo& errinfo) AUG_NOTHROW
    {
        return aug_perrinfo(ctx, s, &errinfo);
    }

    /**
     * Get random number.
     *
     * @return Next random number.
     */

    inline long
    rand(void)
    {
        return aug_rand();
    }

    /**
     * Seed random number generator.
     *
     * @param seed Seed to use.
     */

    inline void
    srand(unsigned seed)
    {
        aug_srand(seed);
    }
}

#endif // AUGSYSPP_UTILITY_HPP
