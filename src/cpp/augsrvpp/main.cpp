/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSRVPP_BUILD
#include "augsrvpp/main.hpp"

#include "augsyspp/exception.hpp"

#include "augsrv/main.h"

#include "augsys/errno.h"
#include "augsys/log.h"

using namespace aug;

namespace {

    int
    setopt_(void* arg, enum aug_option opt, const char* value)
    {
        try {
            service_base* ptr = static_cast<service_base*>(arg);
            ptr->setopt(opt, value);
            return 0;
        } AUG_CATCHRETURN -1;
    }

    const char*
    getopt_(void* arg, enum aug_option opt)
    {
        try {
            service_base* ptr = static_cast<service_base*>(arg);
            return ptr->getopt(opt);
        } AUG_CATCHRETURN 0;
    }

    int
    config_(void* arg, int daemon)
    {
        try {
            service_base* ptr = static_cast<service_base*>(arg);
            ptr->config(daemon ? true : false);
            return 0;
        } AUG_CATCHRETURN -1;
    }

    int
    init_(void* arg)
    {
        try {
            service_base* ptr = static_cast<service_base*>(arg);
            ptr->init();
            return 0;
        } AUG_CATCHRETURN -1;
    }

    int
    run_(void* arg)
    {
        try {
            service_base* ptr = static_cast<service_base*>(arg);
            ptr->run();
            return 0;
        } AUG_CATCHRETURN -1;
    }
}

AUGSRVPP_API
service_base::~service_base() NOTHROW
{
}

AUGSRVPP_API void
aug::main(service_base& service, const char* program, const char* lname,
          const char* sname, const char* admin, int argc, char* argv[])
{
    struct aug_service local = {
        setopt_,
        getopt_,
        config_,
        init_,
        run_,
        program,
        lname,
        sname,
        admin,
        &service
    };

    aug_main(&local, argc, argv);
}
