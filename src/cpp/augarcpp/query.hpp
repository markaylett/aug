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
#ifndef AUGARCPP_QUERY_HPP
#define AUGARCPP_QUERY_HPP

#include "augarcpp/node.hpp"

namespace aug {

    class AUGARCPP_API query_base : public node_base {
    protected:
        virtual match
        do_query(path_iterator pit, path_iterator pend) const = 0;

        virtual bool
        do_isdead() const = 0;

    public:
        ~query_base() AUG_NOTHROW;

        match
        query(path_iterator pit, path_iterator pend) const
        {
            return do_query(pit, pend);
        }
        bool
        isdead() const
        {
            return do_isdead();
        }
    };

    typedef smartptr<query_base> queryptr;
}

#endif // AUGARCPP_QUERY_HPP
