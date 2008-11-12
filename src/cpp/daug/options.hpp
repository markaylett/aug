/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_OPTIONS_HPP
#define DAUG_OPTIONS_HPP

#include <map>
#include <string>

#include "augsyspp/config.hpp"

namespace daug {

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

#endif // DAUG_OPTIONS_HPP
