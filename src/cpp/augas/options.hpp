/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_OPTIONS_HPP
#define AUGAS_OPTIONS_HPP

#include <map>
#include <string>

#include "augsyspp/config.hpp"

namespace augas {

    class options {
        std::map<std::string, std::string> options_;
    public:
        ~options() AUG_NOTHROW;

        void
        read(const char* path);

        void
        set(const std::string& name, const std::string& value);

        const char*
        get(const std::string& name, const char* def) const;

        const std::string&
        get(const std::string& name) const;
    };
}

#endif // AUGAS_OPTIONS_HPP
