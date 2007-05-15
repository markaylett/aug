/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "daug/module.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "daug/exception.hpp"
#include "daug/utility.hpp"

#include "augsys/log.h"

#include <exception>

using namespace aug;
using namespace augas;
using namespace std;

module::~module() AUG_NOTHROW
{
    try {
        AUG_DEBUG2("terminating module: name=[%s]", name_.c_str());
        termfn_();
    } AUG_PERRINFOCATCH;
}

module::module(const string& name, const char* path,
               const struct augas_host& host,
               void (*teardown)(const augas_object*))
    : name_(name),
      lib_(path)
{
    AUG_DEBUG2("resolving symbols in module: name=[%s]", name_.c_str());
    augas_initfn initfn(dlsym<augas_initfn>(lib_, "augas_init"));
    termfn_ = dlsym<augas_termfn>(lib_, "augas_term");

    AUG_DEBUG2("initialising module: name=[%s]", name_.c_str());
    const struct augas_module* ptr(initfn(name_.c_str(), &host));
    if (!ptr)
        throw error(__FILE__, __LINE__, EMODCALL, "augas_init() failed");
    setdefaults(module_, *ptr, teardown);
}

void
module::stop() const AUG_NOTHROW
{
    AUG_DEBUG2("stop()");
    module_.stop_();
}

bool
module::start(augas_serv& serv) const AUG_NOTHROW
{
    AUG_DEBUG2("start(): sname=[%s]", serv.name_);
    return AUGAS_OK == module_.start_(&serv);
}

void
module::reconf() const AUG_NOTHROW
{
    AUG_DEBUG2("reconf()");
    module_.reconf_();
}

void
module::event(const char* from, const char* type, const void* user,
              size_t size) const AUG_NOTHROW
{
    AUG_DEBUG2("event(): from=[%s], type=[%s]", from, type);
    module_.event_(from, type, user, size);
}

void
module::closed(const augas_object& sock) const AUG_NOTHROW
{
    AUG_DEBUG2("closed(): id=[%d]", sock.id_);
    module_.closed_(&sock);
}

void
module::teardown(const augas_object& sock) const AUG_NOTHROW
{
    AUG_DEBUG2("teardown(): id=[%d]", sock.id_);
    module_.teardown_(&sock);
}

bool
module::accept(augas_object& sock, const char* addr,
               unsigned short port) const AUG_NOTHROW
{
    AUG_DEBUG2("accept(): id=[%d], addr=[%s], port=[%u]",
               sock.id_, addr, (unsigned)port);
    return AUGAS_OK == module_.accept_(&sock, addr, port);
}

void
module::connected(augas_object& sock, const char* addr,
                unsigned short port) const AUG_NOTHROW
{
    AUG_DEBUG2("connected(): id=[%d], addr=[%s], port=[%u]",
               sock.id_, addr, (unsigned)port);
    module_.connected_(&sock, addr, port);
}

void
module::data(const augas_object& sock, const char* buf,
             size_t size) const AUG_NOTHROW
{
    AUG_DEBUG2("data(): id=[%d]", sock.id_);
    module_.data_(&sock, buf, size);
}

void
module::rdexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW
{
    AUG_DEBUG2("rdexpire(): id=[%d], ms=[%u]", sock.id_, ms);
    module_.rdexpire_(&sock, &ms);
}

void
module::wrexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW
{
    AUG_DEBUG2("wrexpire(): id=[%d], ms=[%u]", sock.id_, ms);
    module_.wrexpire_(&sock, &ms);
}

void
module::expire(const augas_object& timer, unsigned& ms) const AUG_NOTHROW
{
    AUG_DEBUG2("expire(): id=[%d], ms=[%u]", timer.id_, ms);
    module_.expire_(&timer, &ms);
}

bool
module::authcert(const augas_object& sock, const char* subject,
                 const char* issuer) const AUG_NOTHROW
{
    AUG_DEBUG2("authcert(): id=[%d]", sock.id_);
    return AUGAS_OK == module_.authcert_(&sock, subject, issuer);
}
