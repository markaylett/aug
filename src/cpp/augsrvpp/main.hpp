/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRVPP_MAIN_HPP
#define AUGSRVPP_MAIN_HPP

#include "augsrvpp/config.hpp"

#include "augsrv/types.h"

namespace aug {

    class AUGSRVPP_API service_base {

        virtual const char*
        do_getopt(enum aug_option opt) = 0;

        virtual void
        do_config(const char* conffile, bool daemon) = 0;

        virtual void
        do_init() = 0;

        virtual void
        do_run() = 0;

    public:
        virtual
        ~service_base() NOTHROW;

        const char*
        getopt(enum aug_option opt)
        {
            return do_getopt(opt);
        }

        void
        config(const char* conffile, bool daemon)
        {
            do_config(conffile, daemon);
        }

        void
        init()
        {
            do_init();
        }

        void
        run()
        {
            do_run();
        }
    };

    AUGSRVPP_API void
    main(service_base& service, const char* program, const char* lname,
         const char* sname, const char* admin, int argc, char* argv[]);
}

#endif // AUGSRVPP_MAIN_HPP
