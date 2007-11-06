/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define DAUG_BUILD
#include "daug/module.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "daug/exception.hpp"
#include "daug/utility.hpp"

#include "augsys/log.h"

#include <exception>

using namespace aug;
using namespace augrt;
using namespace std;

module::~module() AUG_NOTHROW
{
    try {
        AUG_DEBUG2("terminating module: name=[%s]", name_.c_str());
        termfn_();
    } AUG_PERRINFOCATCH;
}

module::module(const string& name, const char* path,
               const struct augmod_host& host,
               void (*teardown)(const augmod_object*))
    : name_(name),
      lib_(path)
{
    AUG_DEBUG2("resolving symbols in module: name=[%s]", name_.c_str());
    augmod_initfn initfn(dlsym<augmod_initfn>(lib_, "augmod_init"));
    termfn_ = dlsym<augmod_termfn>(lib_, "augmod_term");

    AUG_DEBUG2("initialising module: name=[%s]", name_.c_str());
    const struct augmod_proxy* ptr(initfn(name_.c_str(), &host));
    if (!ptr)
        throw error(__FILE__, __LINE__, EMODCALL, "augmod_init() failed");
    setdefaults(proxy_, *ptr, teardown);
}

void
module::stop() const AUG_NOTHROW
{
    AUG_DEBUG2("stop()");
    proxy_.stop_();
}

bool
module::start(augmod_session& session) const AUG_NOTHROW
{
    AUG_DEBUG2("start(): sname=[%s]", session.name_);
    return AUGMOD_OK == proxy_.start_(&session);
}

void
module::reconf() const AUG_NOTHROW
{
    AUG_DEBUG2("reconf()");
    proxy_.reconf_();
}

void
module::event(const char* from, const char* type, const void* user,
              size_t size) const AUG_NOTHROW
{
    AUG_DEBUG2("event(): from=[%s], type=[%s]", from, type);
    proxy_.event_(from, type, user, size);
}

void
module::closed(const augmod_object& sock) const AUG_NOTHROW
{
    AUG_DEBUG2("closed(): id=[%d]", sock.id_);
    proxy_.closed_(&sock);
}

void
module::teardown(const augmod_object& sock) const AUG_NOTHROW
{
    AUG_DEBUG2("teardown(): id=[%d]", sock.id_);
    proxy_.teardown_(&sock);
}

bool
module::accepted(augmod_object& sock, const char* addr,
                 unsigned short port) const AUG_NOTHROW
{
    AUG_DEBUG2("accept(): id=[%d], addr=[%s], port=[%u]",
               sock.id_, addr, (unsigned)port);
    return AUGMOD_OK == proxy_.accepted_(&sock, addr, port);
}

void
module::connected(augmod_object& sock, const char* addr,
                  unsigned short port) const AUG_NOTHROW
{
    AUG_DEBUG2("connected(): id=[%d], addr=[%s], port=[%u]",
               sock.id_, addr, (unsigned)port);
    proxy_.connected_(&sock, addr, port);
}

void
module::data(const augmod_object& sock, const char* buf,
             size_t size) const AUG_NOTHROW
{
    AUG_DEBUG2("data(): id=[%d]", sock.id_);
    proxy_.data_(&sock, buf, size);
}

void
module::rdexpire(const augmod_object& sock, unsigned& ms) const AUG_NOTHROW
{
    AUG_DEBUG2("rdexpire(): id=[%d], ms=[%u]", sock.id_, ms);
    proxy_.rdexpire_(&sock, &ms);
}

void
module::wrexpire(const augmod_object& sock, unsigned& ms) const AUG_NOTHROW
{
    AUG_DEBUG2("wrexpire(): id=[%d], ms=[%u]", sock.id_, ms);
    proxy_.wrexpire_(&sock, &ms);
}

void
module::expire(const augmod_object& timer, unsigned& ms) const AUG_NOTHROW
{
    AUG_DEBUG2("expire(): id=[%d], ms=[%u]", timer.id_, ms);
    proxy_.expire_(&timer, &ms);
}

bool
module::authcert(const augmod_object& sock, const char* subject,
                 const char* issuer) const AUG_NOTHROW
{
    AUG_DEBUG2("authcert(): id=[%d]", sock.id_);
    return AUGMOD_OK == proxy_.authcert_(&sock, subject, issuer);
}
