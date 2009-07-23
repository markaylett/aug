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
#ifndef AUGASPP_SESSIONS_HPP
#define AUGASPP_SESSIONS_HPP

#include "augaspp/session.hpp"

#include <map>
#include <string>
#include <vector>

namespace aug {

    class sessions : public mpool_ops {

        typedef std::multimap<std::string, sessionptr> topics;

        std::map<std::string, sessionptr> sessions_;
        topics topics_, tmptopics_;

        sessions(const sessions& rhs);

        sessions&
        operator =(const sessions& rhs);

    public:
        ~sessions() AUG_NOTHROW;

        sessions()
        {
        }
        void
        clear();

        void
        insert(const std::string& name, const sessionptr& session,
               const char* topics);

        void
        reconf() const;

        void
        getsessions(std::vector<sessionptr>& sessions) const;

        sessionptr
        getbyname(const std::string& name) const;

        void
        getbytopic(std::vector<sessionptr>& sessions,
                   const std::string& topic) const;
    };
}

#endif // AUGASPP_SESSIONS_HPP
