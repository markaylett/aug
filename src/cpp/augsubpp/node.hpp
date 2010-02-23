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
#ifndef AUGSUBPP_NODE_HPP
#define AUGSUBPP_NODE_HPP

#include "augsubpp/types.hpp"

#include "augsyspp/smartptr.hpp"

#include <iostream>

namespace aug {

    // Split and normalise path.  An underflow_error will be thrown if the
    // path specifies a parent of the root directory, such as "..".

    AUGSUBPP_API path
    splitpath(const std::string& s);

    class AUGSUBPP_API node_base {
    protected:

        virtual void
        do_clear() = 0;

        virtual unsigned
        do_depth() const = 0;

        virtual bool
        do_ismatch(path_iterator pit, path_iterator pend) const = 0;

        virtual void
        do_print(const std::string& path, std::ostream& os) const = 0;

    public:
        virtual
        ~node_base() AUG_NOTHROW;

        void
        clear()
        {
            do_clear();
        }
        unsigned
        depth() const
        {
            return do_depth();
        }
        bool
        ismatch(path_iterator pit, path_iterator pend) const
        {
            return do_ismatch(pit, pend);
        }
        bool
        ismatch(const std::string& s) const
        {
            const path p(splitpath(s));
            return do_ismatch(p.begin(), p.end());
        }
        void
        print(const std::string& path, std::ostream& os) const
        {
            do_print(path, os);
        }
    };

    typedef smartptr<node_base> nodeptr;

    inline std::ostream&
    operator <<(std::ostream& os, const node_base& node)
    {
        node.print(std::string(), os);
        return os;
    }
}

#endif // AUGSUBPP_NODE_HPP
