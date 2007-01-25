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
    if (!sess->init()) {

        // TODO: leave if event posted.

        sesss_.erase(name); // close() will not be called.
    }
}

void
manager::clear()
{
    idtofd_.clear();
    files_.clear();
    sesss_.clear();

    // Modules not released.
}

void
manager::erase(const file_base& file)
{
    AUG_DEBUG2("removing from manager: id=[%d], fd=[%d]", file.id(),
               file.sfd().get());

    idtofd_.erase(file.id());
    files_.erase(file.sfd().get());
}

void
manager::insert(const fileptr& file)
{
    AUG_DEBUG2("adding to manager: id=[%d], fd=[%d]", file->id(),
               file->sfd().get());

    files_.insert(make_pair(file->sfd().get(), file));
    idtofd_.insert(make_pair(file->id(), file->sfd().get()));
}

void
manager::update(const fileptr& file, fdref prev)
{
    AUG_DEBUG2("updating manager: id=[%d], fd=[%d], prev=[%d]", file->id(),
               file->sfd().get(), prev.get());

    files_.insert(make_pair(file->sfd().get(), file));
    files_.erase(prev.get());

    idtofd_[file->id()] = file->sfd().get();
}

void
manager::load(const char* rundir, const options& options,
              const augas_host& host)
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

                aug_info("loading module: name=[%s]", value.c_str());
                aug::chdir(rundir);
                moduleptr module(new augas::module(value, path.c_str(),
                                                   host));
                it = modules_.insert(make_pair(value, module)).first;
            }

            aug_info("creating session: name=[%s]", name.c_str());
            insert(name, sessptr(new augas::sess(it->second, name.c_str())));
        }

    } else {

        // No service list: assume reasonable defaults.

        aug_info("loading module: name=[%s]", DEFAULT_NAME);
        moduleptr module(new augas::module(DEFAULT_NAME, DEFAULT_MODULE,
                                           host));
        modules_[DEFAULT_NAME] = module;

        aug_info("creating session: name=[%s]", DEFAULT_NAME);
        insert(DEFAULT_NAME,
               sessptr(new augas::sess(module, DEFAULT_NAME)));
    }
}

bool
manager::sendall(mplexer& mplexer, augas_id cid, const char* sname,
                 const char* buf, size_t size)
{
    bool ret(true);

    files::const_iterator it(files_.begin()), end(files_.end());
    for (; it != end; ++it) {

        connptr cptr(smartptr_cast<conn_base>(it->second));
        if (null == cptr)
            continue;

        if (!sendable(*cptr)) {
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
manager::sendself(mplexer& mplexer, augas_id cid, const char* buf,
                  size_t size)
{
    connptr cptr(smartptr_cast<conn_base>(getbyid(cid)));
    if (!sendable(*cptr))
        return false;

    cptr->putsome(mplexer, buf, size);
    return true;
}

void
manager::sendother(mplexer& mplexer, augas_id cid, const char* sname,
                   const char* buf, size_t size)
{
    files::const_iterator it(files_.begin()), end(files_.end());
    for (; it != end; ++it) {

        connptr cptr(smartptr_cast<conn_base>(it->second));
        if (null == cptr)
            continue;

        // Ignore self as well as connections that have been marked for
        // shutdown.

        if (cptr->id() == cid || !sendable(*cptr))
            continue;

        cptr->putsome(mplexer, buf, size);
    }
}

void
manager::teardown()
{
    idtofd::iterator rit(idtofd_.begin()), rend(idtofd_.end());
    while (rit != rend) {

        AUG_DEBUG2("teardown: id=[%d], fd=[%d]", rit->first, rit->second);

        files::iterator it(files_.find(rit->second));
        if (it == files_.end())
            throw error(__FILE__, __LINE__, ESTATE, "file not found: fd=[%d]",
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
        files_.erase(it);
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
        throw error(__FILE__, __LINE__, ESTATE, "file not found: fd=[%d]",
                    fd.get());
    return it->second;
}

fileptr
manager::getbyid(augas_id id) const
{
    idtofd::const_iterator it(idtofd_.find(id));
    if (it == idtofd_.end())
        throw error(__FILE__, __LINE__, ESTATE, "file not found: id=[%d]",
                    id);
    return getbyfd(it->second);
}

sessptr
manager::getsess(const string& name) const
{
    sesss::const_iterator it(sesss_.find(name));
    if (it == sesss_.end())
        throw error(__FILE__, __LINE__, ESTATE,
                    "session not found: sname=[%1%]", name.c_str());
    return it->second;
}

bool
manager::empty() const
{
    return files_.empty();
}
