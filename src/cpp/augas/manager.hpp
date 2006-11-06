/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_MANAGER_HPP
#define AUGAS_MANAGER_HPP

#include "augas/module.hpp"

#include "augsyspp/smartfd.hpp"

#include <map>
#include <string>

namespace augas {

    class options;

    typedef std::map<std::string, moduleptr> modules;

    struct serviceinfo {
        std::string name_;
        aug::smartfd sfd_;
        moduleptr module_;
        serviceinfo()
            : sfd_(null)
        {
        }
        serviceinfo(const std::string& name, const aug::smartfd& sfd,
                    const moduleptr& module)
            : name_(name),
              sfd_(sfd),
              module_(module)
        {
        }
    };

    typedef std::map<int, serviceinfo> services;

    struct manager {

        modules modules_;
        services services_;

        void
        load(const options& options, const augas_host& host);
    };

    void
    reconf(const modules& modules);

    moduleptr
    getmodule(const modules& modules, const std::string& name);
}

#endif // AUGAS_MANAGER_HPP
