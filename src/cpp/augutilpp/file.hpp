/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_FILE_HPP
#define AUGUTILPP_FILE_HPP

#include "augutilpp/var.hpp"

#include "augsyspp/exception.hpp"

#include "augutil/file.h"

#include "augsys/errno.h"
#include "augsys/log.h"

namespace aug {

    class AUGUTILPP_API setopt_base {

        virtual void
        do_setopt(const char* name, const char* value) = 0;

    public:
        virtual
        ~setopt_base() NOTHROW
        {
        }

        void
        setopt(const char* name, const char* value)
        {
            do_setopt(name, value);
        }
    };

    namespace detail {

        inline int
        setopt(const struct aug_var* arg, const char* name, const char* value)
        {
            try {
                setopt_base* ptr = static_cast<
                    setopt_base*>(aug_getvarp(arg));
                ptr->setopt(name, value);
                return 0;
            } AUG_SETERRINFOCATCH;
            return -1;
        }
    }

    inline void
    readconf(const char* path, setopt_base& action)
    {
        var v(&action);
        if (-1 == aug_readconf(path, detail::setopt, cptr(v)))
            throwerrinfo("aug_readconf() failed");
    }
}

#endif // AUGUTILPP_FILE_HPP
