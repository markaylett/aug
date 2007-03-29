/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/manager.hpp"

#include "augas/exception.hpp"
#include "augas/options.hpp"

#include <sstream>

using namespace aug;
using namespace augas;
using namespace std;

namespace {

    const char DEFAULT_NAME[] = "default";

#if !defined(_WIN32)
    const char DEFAULT_MODULE[] = "./modskel.so";
    const char MODEXT[] = ".so";
#else // _WIN32
    const char DEFAULT_MODULE[] = "./modskel.dll";
    const char MODEXT[] = ".dll";
#endif // _WIN32

    bool
    withso(const string& s)
    {
        string::size_type n(s.size());
        return 3 < n
            && '.' == s[n - 3]
            && ('s' == s[n - 2] || 'S' == s[n - 2])
            && ('o' == s[n - 1] || 'O' == s[n - 1]);
    }

    bool
    withdll(const string& s)
    {
        string::size_type n(s.size());
        return 4 < n
            && '.' == s[n - 4]
            && ('d' == s[n - 3] || 'D' == s[n - 3])
            && ('l' == s[n - 2] || 'L' == s[n - 2])
            && ('l' == s[n - 1] || 'L' == s[n - 1]);
    }

    bool
    withext(const string& s)
    {
        return withso(s) || withdll(s);
    }
}

bool
manager::append(mplexer& mplexer, augas_id cid, const aug_var& var)
{
    connptr cptr(smartptr_cast<conn_base>(getbyid(cid)));
    if (!sendable(*cptr))
        return false;

    cptr->append(mplexer, var);
    return true;
}

bool
manager::append(mplexer& mplexer, augas_id cid, const void* buf, size_t len)
{
    connptr cptr(smartptr_cast<conn_base>(getbyid(cid)));
    if (!sendable(*cptr))
        return false;

    cptr->append(mplexer, buf, len);
    return true;
}

void
manager::insert(const string& name, const servptr& serv, const char* groups)
{
    // The service's start() function may callback into the host.  All state
    // will, therefore, need to be configured prior to calling start().  If
    // start() fails then the state changes will need to be rolled-back.  The
    // temp_ container is used to store the groups that need to be removed on
    // failure.

    // The implementation is simplified by adding the service name as a group.

    temp_.insert(make_pair(name, serv));

    if (groups) {

        istringstream is(groups);
        string name;
        while (is >> name)
            temp_.insert(make_pair(name, serv));
    }

    // Insert prior to calling open().

    servs_[name] = serv;
    if (!serv->start()) {

        // TODO: leave if event posted.

        servs_.erase(name); // close() will not be called.

    } else
        groups_.insert(temp_.begin(), temp_.end());

    temp_.clear();
}

void
manager::clear()
{
    idtofd_.clear();
    socks_.clear();
    temp_.clear();
    groups_.clear();

    // TODO: erase the services in reverse order to which they were added.

    servs_.clear();

    // Modules not released.
}

void
manager::erase(const object_base& sock)
{
    AUG_DEBUG2("removing from manager: id=[%d], fd=[%d]", sock.id(),
               sock.sfd().get());

    idtofd_.erase(sock.id());
    socks_.erase(sock.sfd().get());
}

void
manager::insert(const objectptr& sock)
{
    AUG_DEBUG2("adding to manager: id=[%d], fd=[%d]", sock->id(),
               sock->sfd().get());

    socks_.insert(make_pair(sock->sfd().get(), sock));
    idtofd_.insert(make_pair(sock->id(), sock->sfd().get()));
}

void
manager::update(const objectptr& sock, fdref prev)
{
    AUG_DEBUG2("updating manager: id=[%d], fd=[%d], prev=[%d]", sock->id(),
               sock->sfd().get(), prev.get());

    socks_.insert(make_pair(sock->sfd().get(), sock));
    socks_.erase(prev.get());

    idtofd_[sock->id()] = sock->sfd().get();
}

void
manager::load(const char* rundir, const options& options,
              const augas_host& host)
{
    // TODO: allow each service to specify a list of services on which it
    // depends.

    // Obtain list of services.

    const char* value(options.get("services", 0));
    if (value) {

        // For each service...

        istringstream is(value);
        string name, value;
        while (is >> name) {

            // Obtain module associated with service.

            const string base(string("service.").append(name));
            value = options.get(base + ".module");

            modules::iterator it(modules_.find(value));
            if (it == modules_.end()) {

                // Load module.

                string path(options.get(string("module.").append(value)
                                        .append(".path")));
                if (!withext(path))
                    path += MODEXT;

                aug_info("loading module: name=[%s], path=[%s]",
                         value.c_str(), path.c_str());
                aug::chdir(rundir);
                moduleptr module(new augas::module(value, path.c_str(),
                                                   host));
                it = modules_.insert(make_pair(value, module)).first;
            }

            aug_info("creating service: name=[%s]", name.c_str());
            insert(name, servptr(new augas::serv(it->second, name.c_str())),
                   options.get(base + ".groups", 0));
        }

    } else {

        // No service list: assume reasonable defaults.

        aug_info("loading module: name=[%s]", DEFAULT_NAME);
        moduleptr module(new augas::module(DEFAULT_NAME, DEFAULT_MODULE,
                                           host));
        modules_[DEFAULT_NAME] = module;

        aug_info("creating service: name=[%s]", DEFAULT_NAME);
        insert(DEFAULT_NAME,
               servptr(new augas::serv(module, DEFAULT_NAME)), 0);
    }
}

void
manager::teardown()
{
    // Ids are stored in reverse order using the the greater<> predicate.

    idtofd::iterator rit(idtofd_.begin()), rend(idtofd_.end());
    while (rit != rend) {

        AUG_DEBUG2("teardown: id=[%d], fd=[%d]", rit->first, rit->second);

        socks::iterator it(socks_.find(rit->second));
        if (it == socks_.end())
            throw error(__FILE__, __LINE__, ESTATE, "sock not found: fd=[%d]",
                        rit->second);

        connptr cptr(smartptr_cast<conn_base>(it->second));
        if (null != cptr) {
            ++rit;
            try {
                cptr->teardown();
            } AUG_PERRINFOCATCH;
            continue;
        }

        // Not a connection.

        idtofd_.erase(rit++);
        socks_.erase(it);
    }
}

void
manager::reconf() const
{
    servs::const_iterator it(servs_.begin()), end(servs_.end());
    for (; it != end; ++it)
        it->second->reconf();
}

objectptr
manager::getbyfd(fdref fd) const
{
    socks::const_iterator it(socks_.find(fd.get()));
    if (it == socks_.end())
        throw error(__FILE__, __LINE__, ESTATE, "sock not found: fd=[%d]",
                    fd.get());
    return it->second;
}

objectptr
manager::getbyid(augas_id id) const
{
    idtofd::const_iterator it(idtofd_.find(id));
    if (it == idtofd_.end())
        throw error(__FILE__, __LINE__, ESTATE, "sock not found: id=[%d]",
                    id);
    return getbyfd(it->second);
}

servptr
manager::getserv(const string& name) const
{
    servs::const_iterator it(servs_.find(name));
    if (it == servs_.end())
        throw error(__FILE__, __LINE__, ESTATE,
                    "service not found: sname=[%s]", name.c_str());
    return it->second;
}

void
manager::getservs(vector<servptr>& servs, const string& group) const
{
    pair<groups::const_iterator,
        groups::const_iterator> its(groups_.equal_range(group));

    for (; its.first != its.second; ++its.first)
        servs.push_back(its.first->second);

    // Include any groups in the temporary container.

    its = temp_.equal_range(group);

    for (; its.first != its.second; ++its.first)
        servs.push_back(its.first->second);
}

bool
manager::empty() const
{
    return socks_.empty();
}
