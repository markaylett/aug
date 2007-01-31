/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/module.hpp"

#include "augas/exception.hpp"
#include "augas/utility.hpp"

#include "augsys/log.h"

#include <exception>

using namespace aug;
using namespace augas;
using namespace std;

module::~module() AUG_NOTHROW
{
    try {
        AUG_DEBUG2("terminating module: name=[%s]", name_.c_str());
        unloadfn_();
    } AUG_PERRINFOCATCH;
}

module::module(const string& name, const char* path,
               const struct augas_host& host)
    : name_(name),
      lib_(path)
{
    AUG_DEBUG2("resolving symbols in module: name=[%s]", name_.c_str());
    augas_loadfn loadfn(dlsym<augas_loadfn>(lib_, "augas_load"));
    unloadfn_ = dlsym<augas_unloadfn>(lib_, "augas_unload");

    AUG_DEBUG2("initialising module: name=[%s]", name_.c_str());
    const struct augas_module* ptr(loadfn(name_.c_str(), &host));
    if (!ptr)
        throw error(__FILE__, __LINE__, EMODCALL, "augas_load() failed");
    setdefaults(module_, *ptr);
}

void
module::term(const augas_sess& sess) const AUG_NOTHROW
{
    AUG_DEBUG2("term(): sname=[%s]", sess.name_);
    module_.term_(&sess);
}

bool
module::init(augas_sess& sess) const AUG_NOTHROW
{
    AUG_DEBUG2("init(): sname=[%s]", sess.name_);
    return AUGAS_OK == module_.init_(&sess);
}

bool
module::reconf(const augas_sess& sess) const AUG_NOTHROW
{
    AUG_DEBUG2("reconf(): sname=[%s]", sess.name_);
    return AUGAS_OK == module_.reconf_(&sess);
}

bool
module::event(const augas_sess& sess, int type, void* user) const AUG_NOTHROW
{
    AUG_DEBUG2("event(): sname=[%s], type=[%d]", sess.name_, type);
    return AUGAS_OK == module_.event_(&sess, type, user);
}

void
module::closed(const augas_object& sock) const AUG_NOTHROW
{
    AUG_DEBUG2("closed(): sname=[%s], id=[%d]", sock.sess_->name_, sock.id_);
    module_.closed_(&sock);
}

bool
module::teardown(const augas_object& sock) const AUG_NOTHROW
{
    AUG_DEBUG2("teardown(): sname=[%s], id=[%d]",
               sock.sess_->name_, sock.id_);
    return AUGAS_OK == module_.teardown_(&sock);
}

bool
module::accept(augas_object& sock, const char* addr,
               unsigned short port) const AUG_NOTHROW
{
    AUG_DEBUG2("accept(): sname=[%s], id=[%d], addr=[%s], port=[%u]",
               sock.sess_->name_, sock.id_, addr, (unsigned)port);
    return AUGAS_OK == module_.accept_(&sock, addr, port);
}

bool
module::connected(augas_object& sock, const char* addr,
                unsigned short port) const AUG_NOTHROW
{
    AUG_DEBUG2("connected(): sname=[%s], id=[%d], addr=[%s], port=[%u]",
               sock.sess_->name_, sock.id_, addr, (unsigned)port);
    return AUGAS_OK == module_.connected_(&sock, addr, port);
}

bool
module::data(const augas_object& sock, const char* buf,
             size_t size) const AUG_NOTHROW
{
    AUG_DEBUG2("data(): sname=[%s], id=[%d]", sock.sess_->name_, sock.id_);
    return AUGAS_OK == module_.data_(&sock, buf, size);
}

bool
module::rdexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW
{
    AUG_DEBUG2("rdexpire(): sname=[%s], id=[%d], ms=[%u]",
               sock.sess_->name_, sock.id_, ms);
    return AUGAS_OK == module_.rdexpire_(&sock, &ms);
}

bool
module::wrexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW
{
    AUG_DEBUG2("wrexpire(): sname=[%s], id=[%d], ms=[%u]",
               sock.sess_->name_, sock.id_, ms);
    return AUGAS_OK == module_.wrexpire_(&sock, &ms);
}

bool
module::expire(const augas_object& timer, unsigned& ms) const AUG_NOTHROW
{
    AUG_DEBUG2("expire(): sname=[%s], id=[%d], ms=[%u]",
               timer.sess_->name_, timer.id_, ms);
    return AUGAS_OK == module_.expire_(&timer, &ms);
}
