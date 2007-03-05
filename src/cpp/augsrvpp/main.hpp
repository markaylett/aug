/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRVPP_MAIN_HPP
#define AUGSRVPP_MAIN_HPP

#include "augsrvpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augutilpp/var.hpp"

#include "augsys/errno.h"
#include "augsys/log.h"
#include "augsrv/main.h"
#include "augsrv/types.h"

namespace aug {

    class service_base {

        virtual const char*
        do_getopt(enum aug_option opt) = 0;

        virtual void
        do_readconf(const char* conffile, bool daemon) = 0;

        virtual void
        do_init() = 0;

        virtual void
        do_run() = 0;

        virtual void
        do_term() = 0;

    public:
        virtual
        ~service_base() AUG_NOTHROW
        {
        }

        const char*
        getopt(enum aug_option opt)
        {
            return do_getopt(opt);
        }

        void
        readconf(const char* conffile, bool daemon)
        {
            do_readconf(conffile, daemon);
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

        void
        term()
        {
            do_term();
        }
    };

    namespace detail {

        inline const char*
        getopt(const aug_var* arg, enum aug_option opt)
        {
            try {
                service_base* ptr = static_cast<
                    service_base*>(aug_getvarp(arg));
                return ptr->getopt(opt);
            } AUG_SETERRINFOCATCH;
            return 0;
        }

        inline int
        readconf(const aug_var* arg, const char* conffile, int daemon)
        {
            try {
                service_base* ptr = static_cast<
                    service_base*>(aug_getvarp(arg));
                ptr->readconf(conffile, daemon ? true : false);
                return 0;
            } AUG_SETERRINFOCATCH;
            return -1;
        }

        inline int
        init(const aug_var* arg)
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
        run(const aug_var* arg)
        {
            try {
                service_base* ptr = static_cast<
                    service_base*>(aug_getvarp(arg));
                ptr->run();
                return 0;
            } AUG_SETERRINFOCATCH;
            return -1;
        }

        inline void
        term(const aug_var* arg)
        {
            try {
                service_base* ptr = static_cast<
                    service_base*>(aug_getvarp(arg));
                ptr->term();
            } AUG_PERRINFOCATCH;
        }
    }

    /**
       On Windows, the Service Manager calls the service entry point on a
       separate thread - automatic variables on the main thread's stack will
       not be visible from the service thread.  A shallow copy of the service
       structure will be performed by aug_main().
    */

    inline int
    main(int argc, char* argv[], service_base& service)
    {
        static const aug_service local = {
            detail::getopt,
            detail::readconf,
            detail::init,
            detail::run,
            detail::term
        };
        var v(&service);
        return aug_main(argc, argv, &local, cptr(v));
    }
}

#endif // AUGSRVPP_MAIN_HPP
