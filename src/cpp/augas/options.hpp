/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_OPTIONS_HPP
#define AUGAS_OPTIONS_HPP

#include "augutilpp/file.hpp"

#include <map>
#include <string>

namespace augas {

    class options : private aug::confcb_base {
        std::map<std::string, std::string> options_;
        void
        do_callback(const char* name, const char* value);

    public:
        ~options() AUG_NOTHROW;

        void
        read(const char* path);

        void
        set(const std::string& name, const std::string& value);

        const char*
        get(const std::string& name, const char* def = 0) const;
    };
}

#endif // AUGAS_OPTIONS_HPP
