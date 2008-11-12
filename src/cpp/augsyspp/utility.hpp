/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_UTILITY_HPP
#define AUGSYSPP_UTILITY_HPP

#include "augsyspp/types.hpp"

#include "augctxpp/exception.hpp"

#include "augsys/utility.h" // aug_rand()

namespace aug {

    /**
     * Get next process-unique id.
     *
     * @return Next id.
     */

    inline unsigned
    nextid()
    {
        return aug_nextid();
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
