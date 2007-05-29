/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_SERVS_HPP
#define AUGRTPP_SERVS_HPP

#include "augrtpp/serv.hpp"

#include <map>
#include <vector>

namespace aug {

    class servs {

        typedef std::multimap<std::string, servptr> groups;

        std::map<std::string, servptr> servs_;
        groups groups_, tmpgroups_;

        servs(const servs& rhs);

        servs&
        operator =(const servs& rhs);

    public:
        ~servs() AUG_NOTHROW;

        servs()
        {
        }
        void
        clear();

        void
        insert(const std::string& name, const servptr& serv,
               const char* groups);

        void
        reconf() const;

        servptr
        getbyname(const std::string& name) const;

        void
        getbygroup(std::vector<servptr>& servs,
                   const std::string& group) const;
    };
}

#endif // AUGRTPP_SERVS_HPP
