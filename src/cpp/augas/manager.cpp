/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augas/manager.hpp"

#include "augas/exception.hpp"
#include "augas/options.hpp"

#include "augsyspp/endpoint.hpp"
#include "augsyspp/inetaddr.hpp"
#include "augnetpp/inet.hpp"

#include <sstream>

using namespace aug;
using namespace augas;
using namespace std;

namespace {

    const char DEFAULT_LISTEN[] = "0.0.0.0:8080";

#if !defined(_WIN32)
    const char DEFAULT_MODULE[] = "module.so";
#else // _WIN32
    const char DEFAULT_MODULE[] = "module.dll";
#endif // _WIN32

    void
    removeunused(modules& modules)
    {
        map<string, moduleptr>::iterator it(modules.begin()),
            end(modules.end());
        while (it != end) {
            if (1 < it->second.refs())
                modules.erase(it++);
            else
                ++it;
        }
    }

    smartfd
    tcplisten(const char* listen)
    {
        aug_hostserv hostserv;
        parsehostserv(listen, hostserv);
        endpoint ep(null);
        smartfd sfd(tcplisten(hostserv.host_, hostserv.serv_, ep));
        inetaddr addr(null);
        aug_info("listening on interface '%s', port '%d'",
                 inetntop(getinetaddr(ep, addr)).c_str(),
                 static_cast<int>(port(ep)));
        return sfd;
    }
}

void
manager::load(const options& options, const augas_host& host)
{
    // Stop all listener sockets.

    services_.clear();

    // Delete modules that are not currently in use.

    removeunused(modules_);

    // Obtain list of services.

    const char* value(options.get("services", 0));
    if (value) {

        // For each service...

        istringstream is(value);
        string name, value;
        while (is >> name) {

            // Obtain module associated with service.

            string base(string("service.").append(name));
            value = options.get(base + ".module");

            modules::iterator it(modules_.find(value));
            if (it == modules_.end()) {

                // Load module.

                string path(options.get(string("module.").append(value)
                                        .append(".path")));
                moduleptr ptr(new augas::module(value, path.c_str(), host));
                it = modules_.insert(make_pair(value, ptr)).first;
            }

            // Bind listener socket.

            value = options.get(base + ".listen");
            aug_info("binding service '%s'", name.c_str());
            smartfd sfd(tcplisten(value.c_str()));
            services_[sfd.get()] = serviceinfo(name, sfd, it->second);
        }

    } else {

        // No service list: assume reasonable defaults.

        moduleptr ptr(new augas::module("default", DEFAULT_MODULE, host));
        aug_info("binding service 'default'");
        smartfd sfd(tcplisten(DEFAULT_LISTEN));
        modules_["default"] = ptr;
        services_[sfd.get()] = serviceinfo("default", sfd, ptr);
    }
}

void
augas::reconf(const modules& modules)
{
    modules::const_iterator it(modules.begin()), end(modules.end());
    for (; it != end; ++it)
        it->second->reconf();
}

moduleptr
augas::getmodule(const modules& modules, const std::string& name)
{
    modules::const_iterator it(modules.find(name));
    if (it == modules.end())
        throw error(__FILE__, __LINE__, ECONFIG,
                    "module '%s' not found", name.c_str());
    return it->second;
}
