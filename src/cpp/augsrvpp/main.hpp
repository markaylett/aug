/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRVPP_MAIN_HPP
#define AUGSRVPP_MAIN_HPP

#include "augsrvpp/config.hpp"

#include "augctxpp/exception.hpp"

#include "augsrv/main.h"
#include "augsrv/types.h"

#include "augctx/errno.h"

#include "augext/log.h"

namespace aug {

    namespace detail {

        template <typename T>
        class servicestatic {
            static const char*
            getopt(void* arg, enum aug_option opt) AUG_NOTHROW
            {
                try {
                    return T::getopt(arg, opt);
                } AUG_SETERRINFOCATCH;
                return 0;
            }
            static aug_result
            readconf(void* arg, const char* conffile, int batch,
                     int daemon) AUG_NOTHROW
            {
                try {
                    return T::readconf(arg, conffile, batch ? true : false,
                                       daemon ? true : false);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }
            static aug_result
            init(void* arg) AUG_NOTHROW
            {
                try {
                    return T::init(arg);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }
            static aug_result
            run(void* arg) AUG_NOTHROW
            {
                try {
                    return T::run(arg);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }
            static void
            term(void* arg) AUG_NOTHROW
            {
                try {
                    T::term(arg);
                } AUG_PERRINFOCATCH;
            }

        public:
            static const aug_service&
            get()
            {
                static const aug_service local = {
                    getopt,
                    readconf,
                    init,
                    run,
                    term
                };
                return local;
            }
        };

        template <typename T>
        class servicenonstatic {
            static const char*
            getopt(void* arg, enum aug_option opt) AUG_NOTHROW
            {
                try {
                    return static_cast<T*>(arg)->getopt(opt);
                } AUG_SETERRINFOCATCH;
                return 0;
            }
            static aug_result
            readconf(void* arg, const char* conffile, int batch,
                     int daemon) AUG_NOTHROW
            {
                try {
                    return static_cast<T*>(arg)
                        ->readconf(conffile, batch ? true : false,
                                   daemon ? true : false);
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }
            static aug_result
            init(void* arg) AUG_NOTHROW
            {
                try {
                    return static_cast<T*>(arg)->init();
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }
            static aug_result
            run(void* arg) AUG_NOTHROW
            {
                try {
                    return static_cast<T*>(arg)->run();
                } AUG_SETERRINFOCATCH;
                return AUG_FAILERROR;
            }
            static void
            term(void* arg) AUG_NOTHROW
            {
                try {
                    static_cast<T*>(arg)->term();
                } AUG_PERRINFOCATCH;
            }

        public:
            static const aug_service&
            get()
            {
                static const aug_service local = {
                    getopt,
                    readconf,
                    init,
                    run,
                    term
                };
                return local;
            }
        };
    }

    template <typename T>
    const aug_service&
    servicestatic()
    {
        return detail::servicestatic<T>::get();
    }

    template <typename T>
    const aug_service&
    servicenonstatic()
    {
        return detail::servicenonstatic<T>::get();
    }

    /**
     * On Windows, the Service Manager calls the service entry point on a
     * separate thread - automatic variables on the main thread's stack will
     * not be visible from the service thread.  A shallow copy of the service
     * structure is, therefore, performed by aug_main().
     */

    inline int
    main(int argc, char* argv[], const aug_service& service, void* arg)
    {
        return aug_main(argc, argv, &service, arg);
    }

    inline int
    main(int argc, char* argv[], const aug_service& service, const null_&)
    {
        return aug_main(argc, argv, &service, 0);
    }

    template <typename T>
    int
    main(int argc, char* argv[], T& x)
    {
        return aug_main(argc, argv, &servicenonstatic<T>(), &x);
    }
}

#endif // AUGSRVPP_MAIN_HPP
