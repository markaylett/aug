/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_UTILITY_HPP
#define AUGSYSPP_UTILITY_HPP

#include "augsyspp/types.hpp"

#include "augctxpp/exception.hpp"

#include "augsys/unistd.h"  // aug_fsize()
#include "augsys/utility.h" // aug_rand()

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
