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
#ifndef AUGARCPP_INVERSE_HPP
#define AUGARCPP_INVERSE_HPP

#include "augarcpp/types.hpp"

namespace aug {

    class AUGARCPP_API factory_base;

    template <typename baseT>
    class inverse : public baseT {
    protected:

        // node_base

        void
        do_print(const std::string& path, std::ostream& os) const
        {
            baseT::do_print('!' + path, os);
        }

        // inner_base

        void
        do_insertname(const std::string& name)
        {
            baseT::do_erasename(name);
        }

        void
        do_insertname(const std::string& name, path_iterator pit,
                      path_iterator pend)
        {
            baseT::do_erasename(name, pit, pend);
        }

        void
        do_insertrecur()
        {
            baseT::do_eraserecur();
        }

        void
        do_insertwild()
        {
            baseT::do_erasewild();
        }

        void
        do_insertwild(path_iterator pit, path_iterator pend)
        {
            baseT::do_erasewild(pit, pend);
        }

        void
        do_erasename(const std::string& name)
        {
            baseT::do_insertname(name);
        }

        void
        do_erasename(const std::string& name, path_iterator pit,
                     path_iterator pend)
        {
            baseT::do_insertname(name, pit, pend);
        }

        void
        do_eraserecur()
        {
            baseT::do_insertrecur();
        }

        void
        do_erasewild()
        {
            baseT::do_insertwild();
        }

        void
        do_erasewild(path_iterator pit, path_iterator pend)
        {
            baseT::do_insertwild(pit, pend);
        }

    public:

        ~inverse() AUG_NOTHROW
        {
        }

        explicit
        inverse(const factory_base& factory)
            : baseT(factory)
        {
        }
    };
}

#endif // AUGARCPP_INVERSE_HPP
