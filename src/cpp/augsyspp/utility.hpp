/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_UTILITY_HPP
#define AUGSYSPP_UTILITY_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

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
    filesize(fdref ref)
    {
        size_t size;
        verify(aug_filesize(ref.get(), &size));
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

    inline int
    perrinfo(const char* s) AUG_NOTHROW
    {
        return aug_perrinfo(0, s);
    }

    /**
     * Print last error.
     *
     * @param errinfo User supplied error info.
     *
     * @param s String to be prepended.
     *
     * @return -1 on error.
     */

    inline int
    perrinfo(const aug_errinfo& errinfo, const char* s) AUG_NOTHROW
    {
        return aug_perrinfo(&errinfo, s);
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

    /**
     * Set file non-blocking on or off.
     *
     * @param ref File descriptor.
     *
     * @param on On or off.
     */

    inline void
    setnonblock(fdref ref, bool on)
    {
        verify(aug_setnonblock(ref.get(), on ? 1 : 0));
    }
}

#endif // AUGSYSPP_UTILITY_HPP
