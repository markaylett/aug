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
#ifndef AUGSUBPP_NAMES_HPP
#define AUGSUBPP_NAMES_HPP

#include "augsubpp/outer.hpp"
#include "augsubpp/typed.hpp"

#include <map>

namespace aug {

    class AUGSUBPP_API factory_base;

    class names : public typed_base {

        typedef std::map<std::string, outerptr> nodes;

        const factory_base& factory_;
        nodes nodes_;

    protected:

        // node_base

        void
        do_clear();

        unsigned
        do_depth() const;

        bool
        do_ismatch(path_iterator pit, path_iterator pend) const;

        void
        do_print(const std::string& path, std::ostream& os) const;

        // query_base

        match
        do_query(path_iterator pit, path_iterator pend) const;

        bool
        do_isdead() const;

        // inner_base

        void
        do_insertname(const std::string& name);

        void
        do_insertname(const std::string& name, path_iterator pit,
                      path_iterator pend);

        void
        do_insertrecur();

        void
        do_insertwild();

        void
        do_insertwild(path_iterator pit, path_iterator pend);

        void
        do_erasename(const std::string& name);

        void
        do_erasename(const std::string& name, path_iterator pit,
                     path_iterator pend);

        void
        do_eraserecur();

        void
        do_erasewild();

        void
        do_erasewild(path_iterator pit, path_iterator pend);

        // typed_base

        nodetype
        do_type() const;

    public:

        ~names() AUG_NOTHROW;

        explicit
        names(const factory_base& factory)
            : factory_(factory)
        {
        }
    };
}

#endif // AUGSUBPP_NAMES_HPP
