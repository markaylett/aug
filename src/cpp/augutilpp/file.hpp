/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_FILE_HPP
#define AUGUTILPP_FILE_HPP

#include "augutilpp/config.hpp"

#include "augutil/file.h"

namespace aug {

    class AUGUTILPP_API setopt_base {

        virtual void
        do_setopt(const char* name, const char* value) = 0;

    public:
        virtual
        ~setopt_base() NOTHROW;

        void
        setopt(const char* name, const char* value)
        {
            do_setopt(name, value);
        }
    };

    AUGUTILPP_API void
    readconf(const char* path, setopt_base& action);
}

#endif // AUGUTILPP_FILE_HPP
