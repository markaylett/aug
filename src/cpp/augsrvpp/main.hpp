/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRVPP_MAIN_HPP
#define AUGSRVPP_MAIN_HPP

#include "augsrvpp/config.hpp"

#include "augsyspp/exception.hpp"

#include "augsrv/main.h"
#include "augsrv/types.h"

#include "augsys/errno.h"
#include "augsys/log.h"

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
        ~service_base() NOTHROW
        {
        }

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

    namespace detail {

        inline const char*
        getopt(void* arg, enum aug_option opt)
        {
            try {
                service_base* ptr = static_cast<service_base*>(arg);
                return ptr->getopt(opt);
            } AUG_CATCHRETURN 0;
        }

        inline int
        config(void* arg, const char* conffile, int daemon)
        {
            try {
                service_base* ptr = static_cast<service_base*>(arg);
                ptr->config(conffile, daemon ? true : false);
                return 0;
            } AUG_CATCHRETURN -1;
        }

        inline int
        init(void* arg)
        {
            try {
                service_base* ptr = static_cast<service_base*>(arg);
                ptr->init();
                return 0;
            } AUG_CATCHRETURN -1;
        }

        inline int
        run(void* arg)
        {
            try {
                service_base* ptr = static_cast<service_base*>(arg);
                ptr->run();
                return 0;
            } AUG_CATCHRETURN -1;
        }
    }

    inline void
    main(service_base& service, const char* program, const char* lname,
         const char* sname, const char* admin, int argc, char* argv[])
    {
        struct aug_service s = {
            detail::getopt,
            detail::config,
            detail::init,
            detail::run,
            program,
            lname,
            sname,
            admin,
            &service
        };

        aug_main(&s, argc, argv);
    }
}

#endif // AUGSRVPP_MAIN_HPP
