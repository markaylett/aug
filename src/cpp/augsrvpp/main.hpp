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

    namespace detail {

        template <typename T>
        class serverstatic {
            static const char*
            getopt(void* arg, enum aug_option opt) AUG_NOTHROW
            {
                try {
                    return T::getopt(arg, opt);
                } AUG_SETERRINFOCATCH;
                return 0;
            }
            static int
            readconf(void* arg, const char* conffile, int prompt,
                     int daemon) AUG_NOTHROW
            {
                try {
                    T::readconf(arg, conffile, prompt ? true : false,
                                daemon ? true : false);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            init(void* arg) AUG_NOTHROW
            {
                try {
                    T::init(arg);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            run(void* arg) AUG_NOTHROW
            {
                try {
                    T::run(arg);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static void
            term(void* arg) AUG_NOTHROW
            {
                try {
                    T::term(arg);
                } AUG_PERRINFOCATCH;
            }

        public:
            static const aug_server&
            get()
            {
                static const aug_server local = {
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
        class servernonstatic {
            static const char*
            getopt(void* arg, enum aug_option opt) AUG_NOTHROW
            {
                try {
                    static_cast<T*>(arg)->getopt(opt);
                } AUG_SETERRINFOCATCH;
                return 0;
            }
            static int
            readconf(void* arg, const char* conffile, int prompt,
                     int daemon) AUG_NOTHROW
            {
                try {
                    static_cast<T*>(arg)
                        ->readconf(conffile, prompt ? true : false,
                                   daemon ? true : false);
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            init(void* arg) AUG_NOTHROW
            {
                try {
                    static_cast<T*>(arg)->init();
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static int
            run(void* arg) AUG_NOTHROW
            {
                try {
                    static_cast<T*>(arg)->run();
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static void
            term(void* arg) AUG_NOTHROW
            {
                try {
                    static_cast<T*>(arg)->term();
                } AUG_PERRINFOCATCH;
            }

        public:
            static const aug_server&
            get()
            {
                static const aug_server local = {
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
    const aug_server&
    serverstatic()
    {
        return detail::serverstatic<T>::get();
    }

    template <typename T>
    const aug_server&
    servernonstatic()
    {
        return detail::servernonstatic<T>::get();
    }

    /**
       On Windows, the Service Manager calls the service entry point on a
       separate thread - automatic variables on the main thread's stack will
       not be visible from the service thread.  A shallow copy of the server
       structure is, therefore, performed by aug_main().
    */

    inline int
    main(int argc, char* argv[], const aug_server& server, void* arg)
    {
        return aug_main(argc, argv, &server, arg);
    }

    inline int
    main(int argc, char* argv[], const aug_server& server, const null_&)
    {
        return aug_main(argc, argv, &server, 0);
    }

    template <typename T>
    int
    main(int argc, char* argv[], T& x)
    {
        return aug_main(argc, argv, &servernonstatic<T>(), &x);
    }
}

#endif // AUGSRVPP_MAIN_HPP
