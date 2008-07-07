/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGASPP_SESSIONS_HPP
#define AUGASPP_SESSIONS_HPP

#include "augaspp/session.hpp"

#include <map>
#include <string>
#include <vector>

namespace aug {

    class sessions {

        typedef std::multimap<std::string, sessionptr> groups;

        std::map<std::string, sessionptr> sessions_;
        groups groups_, tmpgroups_;

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
               const char* groups);

        void
        reconf() const;

        sessionptr
        getbyname(const std::string& name) const;

        void
        getbygroup(std::vector<sessionptr>& sessions,
                   const std::string& group) const;
    };
}

#endif // AUGASPP_SESSIONS_HPP
