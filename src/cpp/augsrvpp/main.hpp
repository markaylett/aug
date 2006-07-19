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

    class service_base {

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
        getopt(const struct aug_var* arg, enum aug_option opt)
        {
            try {
                service_base* ptr = static_cast<
                    service_base*>(aug_getvarp(arg));
                return ptr->getopt(opt);
            } AUG_SETERRINFOCATCH;
            return 0;
        }

        inline int
        config(const struct aug_var* arg, const char* conffile, int daemon)
        {
            try {
                service_base* ptr = static_cast<
                    service_base*>(aug_getvarp(arg));
                ptr->config(conffile, daemon ? true : false);
                return 0;
            } AUG_SETERRINFOCATCH;
            return -1;
        }

        inline int
        init(const struct aug_var* arg)
        {
            try {
                service_base* ptr = static_cast<
                    service_base*>(aug_getvarp(arg));
                ptr->init();
                return 0;
            } AUG_SETERRINFOCATCH;
            return -1;
        }

        inline int
        run(const struct aug_var* arg)
        {
            try {
                service_base* ptr = static_cast<
                    service_base*>(aug_getvarp(arg));
                ptr->run();
                return 0;
            } AUG_SETERRINFOCATCH;
            return -1;
        }
    }

    /** On Windows, the Service Manager calls the service entry point on a
        separate thread - automatic variables on the main thread's stack will
        not be visible from the service thread.  A shallow copy of the service
        structure will be performed by aug_main(). */

    inline void
    main(service_base& service, int argc, char* argv[])
    {
        struct aug_service s = {
            detail::getopt,
            detail::config,
            detail::init,
            detail::run,
            { AUG_VTNULL }
        };
        aug_setvarp(&s.arg_, &service);
        aug_main(&s, argc, argv);
    }
}

#endif // AUGSRVPP_MAIN_HPP
