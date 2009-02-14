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
