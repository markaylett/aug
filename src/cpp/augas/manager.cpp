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
#else // _WIN32
    const char DEFAULT_MODULE[] = "./modskel.dll";
#endif // _WIN32

}

void
manager::insert(const string& name, const sessptr& sess)
{
    // Insert prior to calling open().

    sesss_[name] = sess;
    try {
        try {
            sess->open();
        } catch (...) {

            // TODO: leave if event posted.

            sesss_.erase(name); // close() will not be called.
            throw;
        }
    } AUG_PERRINFOCATCH;
}

void
manager::clear()
{
    idtofd_.clear();
    files_.clear();
    sesss_.clear();
    modules_.clear();
}

void
manager::erase(const file_base& file)
{
    idtofd_.erase(file.id());
    files_.erase(file.fd());
}

void
manager::insert(const fileptr& file)
{
    files_.insert(make_pair(file->fd(), file));
    idtofd_.insert(make_pair(file->id(), file->fd()));
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

                string path(options.get(string("module.").append(value)
                                        .append(".path")));

                aug_info("loading module '%s'", value.c_str());
                moduleptr module(new augas::module(value, path.c_str(),
                                                   host));
                it = modules_.insert(make_pair(value, module)).first;
            }

            aug_info("creating session '%s'", name.c_str());
            insert(name, sessptr(new augas::sess(it->second, name.c_str())));
        }

    } else {

        // No service list: assume reasonable defaults.

        aug_info("loading module '%s'", DEFAULT_NAME);
        moduleptr module(new augas::module(DEFAULT_NAME, DEFAULT_MODULE,
                                           host));
        modules_[DEFAULT_NAME] = module;

        aug_info("creating session '%s'", DEFAULT_NAME);
        insert(DEFAULT_NAME,
               sessptr(new augas::sess(module, DEFAULT_NAME)));
    }
}

bool
manager::sendall(aug::mplexer& mplexer, augas_id cid, const char* sname,
                 const char* buf, size_t size)
{
    bool ret(true);

    files::const_iterator it(files_.begin()), end(files_.end());
    for (; it != end; ++it) {

        connptr cptr(smartptr_cast<conn>(it->second));
        if (null == cptr)
            continue;

        if (cptr->isshutdown()) {
            if (cptr->id() == cid)
                ret = false;
            continue;
        }

        if (cptr->sess()->name() == sname)
            cptr->putsome(mplexer, buf, size);
    }

    return ret;
}

bool
manager::sendself(aug::mplexer& mplexer, augas_id cid, const char* buf,
                  size_t size)
{
    connptr cptr(smartptr_cast<conn>(getbyid(cid)));
    if (cptr->isshutdown())
        return false;

    cptr->putsome(mplexer, buf, size);
    return true;
}

void
manager::sendother(aug::mplexer& mplexer, augas_id cid, const char* sname,
                   const char* buf, size_t size)
{
    files::const_iterator it(files_.begin()), end(files_.end());
    for (; it != end; ++it) {

        connptr cptr(smartptr_cast<conn>(it->second));
        if (null == cptr)
            continue;

        // Ignore self as well as connections that have been marked for
        // shutdown.

        if (cptr->id() == cid || cptr->isshutdown())
            continue;

        cptr->putsome(mplexer, buf, size);
    }
}

void
manager::teardown()
{
    files::iterator it(files_.begin()), end(files_.end());
    while (it != end) {

        connptr cptr(smartptr_cast<conn>(it->second));
        if (null != cptr) {
            cptr->teardown();
            ++it;
            continue;
        }

        // Erase listener.

        files_.erase(it++);
    }
}

void
manager::reconf() const
{
    sesss::const_iterator it(sesss_.begin()), end(sesss_.end());
    for (; it != end; ++it)
        it->second->reconf();
}

fileptr
manager::getbyfd(fdref fd) const
{
    files::const_iterator it(files_.find(fd.get()));
    if (it == files_.end())
        throw error(__FILE__, __LINE__, ESTATE, "fd '%d' not found",
                    fd.get());
    return it->second;
}

fileptr
manager::getbyid(augas_id id) const
{
    idtofd::const_iterator it(idtofd_.find(id));
    if (it == idtofd_.end())
        throw error(__FILE__, __LINE__, ESTATE, "id '%d' not found", id);
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
manager::empty() const
{
    return files_.empty();
}
