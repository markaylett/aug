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
#ifndef AUGUTILPP_CONV_HPP
#define AUGUTILPP_CONV_HPP

#include "augutilpp/config.hpp"

#include "augctxpp/exception.hpp"

#include "augutil/conv.h"

#include <string>

namespace aug {

    inline unsigned long
    strtoul(const char* src, int base)
    {
        unsigned long ul;
        verify(aug_strtoul(&ul, src, base));
        return ul;
    }

    inline unsigned
    strtoui(const char* src, int base)
    {
        unsigned ui;
        verify(aug_strtoui(&ui, src, base));
        return ui;
    }

    inline unsigned short
    strtous(const char* src, int base)
    {
        unsigned short us;
        verify(aug_strtous(&us, src, base));
        return us;
    }
}

#endif // AUGUTILPP_CONV_HPP
