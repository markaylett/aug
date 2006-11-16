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
}

sess::~sess() AUG_NOTHROW
{
    try {
        if (open_)
            module_->closesess(sess_);
    } AUG_PERRINFOCATCH;
}

sess::sess(const moduleptr& module, const char* name)
    : module_(module),
      open_(false)
{
    aug_strlcpy(sess_.name_, name, sizeof(sess_.name_));
    sess_.user_ = 0;
}

void
sess::open()
{
    if (!open_) {
        module_->opensess(sess_);
        open_ = true;
    }
}

void
conn::do_callback(idref ref, unsigned& ms, aug_timers& timers)
{
    if (rdtimer_ == ref) {
        aug_info("read timer expiry");
        sess_->rdexpire(conn_, ms);
    } else if (wrtimer_ == ref) {
        aug_info("write timer expiry");
        sess_->wrexpire(conn_, ms);
    } else
        assert(0);
}

conn::~conn() AUG_NOTHROW
{
    try {
        sess_->closeconn(conn_);
    } AUG_PERRINFOCATCH;
}

conn::conn(const sessptr& sess, const smartfd& sfd, augas_id cid,
           const aug_endpoint& ep, timers& timers)
    : sess_(sess),
      sfd_(sfd),
      rdtimer_(timers, null),
      wrtimer_(timers, null),
      shutdown_(false)
{
    conn_.id_ = cid;
    conn_.user_ = 0;

    inetaddr addr(null);
    sess_->openconn(conn_, inetntop(getinetaddr(ep, addr)).c_str(),
                    port(ep));
    conn_.id_ = cid; // Just in case the callee has modified.
}

bool
conn::process(mplexer& mplexer)
{
    unsigned short bits(ioevents(mplexer, sfd_));

    if (bits & AUG_IOEVENTRD) {

        AUG_DEBUG("handling read event '%d'", sfd_.get());

        char buf[4096];
        size_t size(aug::read(sfd_, buf, sizeof(buf)));
        if (0 == size) {

            // Connection closed.

            aug_info("closing connection '%d'", sfd_.get());
            return false;
        }

        // Data has been read: reset read timer.

        if (null != rdtimer_)
            if (!rdtimer_.reset()) // If timer nolonger exists.
                rdtimer_ = null;

        // Notify module of new data.

        data(buf, size);
    }

    if (bits & AUG_IOEVENTWR) {

        bool more(buffer_.writesome(sfd_));

        // Data has been written: reset write timer.

        if (null != wrtimer_)
            if (!wrtimer_.reset()) // If timer nolonger exists.
                wrtimer_ = null;

        if (!more) {

            // No more (buffered) data to be written.

            setioeventmask(mplexer, sfd_, AUG_IOEVENTRD);

            // If flagged for shutdown, send FIN and disable writes.

            if (shutdown_)
                aug::shutdown(sfd_, SHUT_WR);
        }
    }

    return true;
}

void
manager::erase(const connptr& conn)
{
    conns_.erase(conn->fd());
    idtofd_.erase(conn->id());
}

void
manager::insert(const connptr& conn)
{
    idtofd_.insert(make_pair(conn->id(), conn->fd()));
    conns_.insert(make_pair(conn->fd(), conn));
}

void
manager::insert(const sessptr& sess, const aug::smartfd& sfd)
{
    listeners_.insert(make_pair(sfd.get(), make_pair(sess, sfd)));
}

void
manager::teardown()
{
    conns::const_iterator it(conns_.begin()),
        end(conns_.end());
    for (; it != end; ++it)
        it->second->teardown();
}

void
manager::load(const options& options, const augas_host& host)
{
    // Delete modules that are not currently in use.

    removeunused(modules_);

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
                moduleptr module(new augas::module(value, path.c_str(),
                                                   host));
                it = modules_.insert(make_pair(value, module)).first;
            }

            sessptr sess(new augas::sess(it->second, name.c_str()));

            // Insert before calling open().

            sesss_[name] = sess;

            // TODO: try/catch.

            sess->open();
        }

    } else {

        // No service list: assume reasonable defaults.

        moduleptr module(new augas::module(DEFAULT_NAME, DEFAULT_MODULE,
                                           host));
        modules_[DEFAULT_NAME] = module;
        sessptr sess(new augas::sess(module, DEFAULT_NAME));
        sesss_[DEFAULT_NAME] = sess;
        sess->open();
    }
}

bool
manager::sendall(aug::mplexer& mplexer, augas_id cid, const char* sname,
                 const char* buf, size_t size)
{
    bool ret(true);

    conns::const_iterator it(conns_.begin()),
        end(conns_.end());
    for (; it != end; ++it) {
        if (it->second->isshutdown()) {
            if (it->second->id() == cid)
                ret = false;
            continue;
        }
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
    conns::const_iterator it(conns_.begin()),
        end(conns_.end());
    for (; it != end; ++it) {

        // Ignore self as well as connections that have been marked for
        // shutdown.

        if (it->second->id() == cid || it->second->isshutdown())
            continue;

        it->second->putsome(mplexer, buf, size);
    }
}

connptr
manager::getbyfd(fdref fd) const
{
    conns::const_iterator it(conns_.find(fd.get()));
    if (it != conns_.end())
        throw error(__FILE__, __LINE__, ESTATE,
                    "connection-fd '%d' not found", fd.get());
    return it->second;
}

connptr
manager::getbyid(augas_id id) const
{
    idtofd::const_iterator it(idtofd_.find(id));
    if (it != idtofd_.end())
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

void
manager::reconf() const
{
    sesss::const_iterator it(sesss_.begin()), end(sesss_.end());
    for (; it != end; ++it)
        it->second->reconf();
}


