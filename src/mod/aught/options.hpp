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
#ifndef AUGHT_OPTIONS_HPP
#define AUGHT_OPTIONS_HPP

#include "augarcpp/base.hpp"

#include <map>
#include <string>

namespace aug {

    class options {

        std::string css_;
        std::map<std::string, std::string> mimetypes_;
        std::map<std::pair<std::string, std::string>, std::string> passwd_;
        rootptr services_;

    public:

        void
        load();

        const std::string&
        css() const
        {
            return css_;
        }

        std::string
        mimetype(const std::string& path) const;

        bool
        passwd(const std::string& user, const std::string& realm,
               std::string& digest);

        bool
        service(const std::string& name) const
        {
            return null != services_ && services_->ismatch(name);
        }
    };
}

#endif // AUGHT_OPTIONS_HPP
