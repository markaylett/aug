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
#ifndef AUGSUBPP_OUTER_HPP
#define AUGSUBPP_OUTER_HPP

#include "augsubpp/query.hpp"

namespace aug {

    class AUGSUBPP_API outer_base : public query_base {
    protected:

        virtual void
        do_insert(const std::string& head, path_iterator pit,
                  path_iterator pend) = 0;

        virtual void
        do_insert(const std::string& path) = 0;

        virtual void
        do_erase(const std::string& head, path_iterator pit,
                 path_iterator pend) = 0;

        virtual void
        do_erase(const std::string& path) = 0;

        virtual void
        do_setmatch(match m) = 0;

    public:
        ~outer_base() AUG_NOTHROW;

        void
        insert(const std::string& name, path_iterator pit, path_iterator pend)
        {
            do_insert(name, pit, pend);
        }
        void
        insert(const std::string& path)
        {
            do_insert(path);
        }
        void
        erase(const std::string& name, path_iterator pit, path_iterator pend)
        {
            do_erase(name, pit, pend);
        }
        void
        erase(const std::string& path)
        {
            do_erase(path);
        }
        void
        setmatch(match m)
        {
            do_setmatch(m);
        }
    };

    typedef smartptr<outer_base> outerptr;
}

#endif // AUGSUBPP_OUTER_HPP
