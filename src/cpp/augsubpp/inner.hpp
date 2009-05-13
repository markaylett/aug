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
#ifndef AUGSUBPP_INNER_HPP
#define AUGSUBPP_INNER_HPP

#include "augsubpp/query.hpp"

namespace aug {

    class AUGSUBPP_API inner_base : public query_base {
    protected:

        virtual void
        do_insertname(const std::string& name) = 0;

        virtual void
        do_insertname(const std::string& name, path_iterator pit,
                      path_iterator pend) = 0;

        virtual void
        do_insertrecur() = 0;

        virtual void
        do_insertwild() = 0;

        virtual void
        do_insertwild(path_iterator pit, path_iterator pend) = 0;

        virtual void
        do_erasename(const std::string& name) = 0;

        virtual void
        do_erasename(const std::string& name, path_iterator pit,
                     path_iterator pend) = 0;

        virtual void
        do_eraserecur() = 0;

        virtual void
        do_erasewild() = 0;

        virtual void
        do_erasewild(path_iterator pit, path_iterator pend) = 0;

    public:
        ~inner_base() AUG_NOTHROW;

        void
        insertname(const std::string& name)
        {
            do_insertname(name);
        }
        void
        insertname(const std::string& name, path_iterator pit,
                    path_iterator pend)
        {
            do_insertname(name, pit, pend);
        }
        void
        insertrecur()
        {
            do_insertrecur();
        }
        void
        insertwild()
        {
            do_insertwild();
        }
        void
        insertwild(path_iterator pit, path_iterator pend)
        {
            do_insertwild(pit, pend);
        }
        void
        erasename(const std::string& name)
        {
            do_erasename(name);
        }
        void
        erasename(const std::string& name, path_iterator pit,
                   path_iterator pend)
        {
            do_erasename(name, pit, pend);
        }
        void
        eraserecur()
        {
            do_eraserecur();
        }
        void
        erasewild()
        {
            do_erasewild();
        }
        void
        erasewild(path_iterator pit, path_iterator pend)
        {
            do_erasewild(pit, pend);
        }
    };

    typedef smartptr<inner_base> innerptr;
}

#endif // AUGSUBPP_INNER_HPP
