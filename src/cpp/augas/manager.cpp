/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
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
#else // _WIN32
    const char DEFAULT_MODULE[] = "./modskel.dll";
#endif // _WIN32

}

void
manager::clear()
{
    idtofd_.clear();
    conns_.clear();
    listeners_.clear();
    sesss_.clear();
    modules_.clear();
}

void
manager::erase(const connptr& conn)
{
    idtofd_.erase(conn->id());
    conns_.erase(conn->fd());
}

void
manager::insert(const connptr& conn)
{
    conns_.insert(make_pair(conn->fd(), conn));
    idtofd_.insert(make_pair(conn->id(), conn->fd()));
}

void
manager::insert(const sessptr& sess, const aug::smartfd& sfd)
{
    listeners_.insert(make_pair(sfd.get(), make_pair(sess, sfd)));
}

void
manager::load(const options& options, const augas_host& host)
{
    // TODO: allow each session to specify a list of sessions on which it
    // depends.

    // Obtain list of sessions.

    const char* value(options.get("sessions", 0));
    if (value) {

        // For each session...

        istringstream is(value);
        string name, value;
        while (is >> name) {

            // Obtain module associated with service.

            string base(string("session.").append(name));
            value = options.get(base + ".module");

            modules::iterator it(modules_.find(value));
            if (it == modules_.end()) {

                // Load module.

                aug_info("loading module '%s'", value.c_str());
                string path(options.get(string("module.").append(value)
                                        .append(".path")));
                moduleptr module(new augas::module(value, path.c_str(),
                                                   host));
                it = modules_.insert(make_pair(value, module)).first;
            }

            aug_info("creating session '%s'", name.c_str());

            sessptr sess(new augas::sess(it->second, name.c_str()));

            // Insert before calling open().

            sesss_[name] = sess;
            try {
                try {
                    sess->open();
                } catch (...) {
                    // TODO: leave if event posted.
                    sesss_.erase(name);
                }
            } AUG_PERRINFOCATCH;
        }

    } else {

        // No service list: assume reasonable defaults.

        moduleptr module(new augas::module(DEFAULT_NAME, DEFAULT_MODULE,
                                           host));
        modules_[DEFAULT_NAME] = module;
        sessptr sess(new augas::sess(module, DEFAULT_NAME));
        sesss_[DEFAULT_NAME] = sess;
        try {
            try {
                sess->open();
            } catch (...) {
                // TODO: leave if event posted.
                sesss_.erase(DEFAULT_NAME);
            }
        } AUG_PERRINFOCATCH;
    }
}

bool
manager::sendall(aug::mplexer& mplexer, augas_id cid, const char* sname,
                 const char* buf, size_t size)
{
    bool ret(true);

    conns::const_iterator it(conns_.begin()), end(conns_.end());
    for (; it != end; ++it) {
        if (it->second->isshutdown()) {
            if (it->second->id() == cid)
                ret = false;
            continue;
        }
        if (it->second->sname() == sname)
            it->second->putsome(mplexer, buf, size);
    }

    return ret;
}

bool
manager::sendself(aug::mplexer& mplexer, augas_id cid, const char* buf,
                  size_t size)
{
    connptr cptr(getbyid(cid));
    if (cptr->isshutdown())
        return false;

    cptr->putsome(mplexer, buf, size);
    return true;
}

void
manager::sendother(aug::mplexer& mplexer, augas_id cid, const char* sname,
                   const char* buf, size_t size)
{
    conns::const_iterator it(conns_.begin()), end(conns_.end());
    for (; it != end; ++it) {

        // Ignore self as well as connections that have been marked for
        // shutdown.

        if (it->second->id() == cid || it->second->isshutdown())
            continue;

        it->second->putsome(mplexer, buf, size);
    }
}

void
manager::reconf() const
{
    sesss::const_iterator it(sesss_.begin()), end(sesss_.end());
    for (; it != end; ++it)
        it->second->reconf();
}

void
manager::teardown() const
{
    conns::const_iterator it(conns_.begin()), end(conns_.end());
    for (; it != end; ++it)
        it->second->teardown();
}

connptr
manager::getbyfd(fdref fd) const
{
    conns::const_iterator it(conns_.find(fd.get()));
    if (it == conns_.end())
        throw error(__FILE__, __LINE__, ESTATE,
                    "connection-fd '%d' not found", fd.get());
    return it->second;
}

connptr
manager::getbyid(augas_id id) const
{
    idtofd::const_iterator it(idtofd_.find(id));
    if (it == idtofd_.end())
        throw error(__FILE__, __LINE__, ESTATE,
                    "connection-id '%d' not found", id);
    return getbyfd(it->second);
}

sessptr
manager::getsess(const std::string& name) const
{
    sesss::const_iterator it(sesss_.find(name));
    if (it == sesss_.end())
        throw error(__FILE__, __LINE__, ESTATE,
                    "session '%s' not found", name.c_str());
    return it->second;
}

bool
manager::isconnected() const
{
    return !conns_.empty();
}

sessptr
manager::islistener(aug::fdref fd) const
{
    sessptr sess;
    listeners::const_iterator it(listeners_.find(fd.get()));
    if (it != listeners_.end())
        sess = it->second.first;
    return sess;
}
