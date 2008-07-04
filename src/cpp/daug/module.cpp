/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define DAUG_BUILD
#include "daug/module.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "daug/exception.hpp"
#include "daug/utility.hpp"

#include "augext/log.h"

#include <exception>

using namespace aug;
using namespace aug;
using namespace daug;
using namespace std;

module::~module() AUG_NOTHROW
{
    try {
        AUG_CTXDEBUG2(aug_tlx, "terminating module: name=[%s]",
                      name_.c_str());
        termfn_();
    } AUG_PERRINFOCATCH;
}

module::module(const string& name, const char* path,
               const struct mod_host& host,
               void (*teardown)(const mod_handle*))
    : name_(name),
      lib_(getmpool(aug_tlx), path)
{
    AUG_CTXDEBUG2(aug_tlx, "resolving symbols in module: name=[%s]",
                  name_.c_str());
    mod_initfn initfn(dlsym<mod_initfn>(lib_, "mod_init"));
    termfn_ = dlsym<mod_termfn>(lib_, "mod_term");

    AUG_CTXDEBUG2(aug_tlx, "initialising module: name=[%s]", name_.c_str());
    const struct mod_module* ptr(initfn(name_.c_str(), &host));
    if (!ptr)
        throw daug_error(__FILE__, __LINE__, EMODCALL, "mod_init() failed");
    setdefaults(module_, *ptr, teardown);
}

void
module::stop() const AUG_NOTHROW
{
    AUG_CTXDEBUG2(aug_tlx, "stop()");
    module_.stop_();
}

bool
module::start(mod_session& session) const AUG_NOTHROW
{
    AUG_CTXDEBUG2(aug_tlx, "start(): sname=[%s]", session.name_);
    return MOD_OK == module_.start_(&session);
}

void
module::reconf() const AUG_NOTHROW
{
    AUG_CTXDEBUG2(aug_tlx, "reconf()");
    module_.reconf_();
}

void
module::event(const char* from, const char* type,
              objectref ob) const AUG_NOTHROW
{
    AUG_CTXDEBUG2(aug_tlx, "event(): from=[%s], type=[%s]", from, type);
    module_.event_(from, type, ob.get());
}

void
module::closed(const mod_handle& sock) const AUG_NOTHROW
{
    AUG_CTXDEBUG2(aug_tlx, "closed(): id=[%u]", sock.id_);
    module_.closed_(&sock);
}

void
module::teardown(const mod_handle& sock) const AUG_NOTHROW
{
    AUG_CTXDEBUG2(aug_tlx, "teardown(): id=[%u]", sock.id_);
    module_.teardown_(&sock);
}

bool
module::accepted(mod_handle& sock, const char* name) const AUG_NOTHROW
{
    AUG_CTXDEBUG2(aug_tlx, "accept(): id=[%u], name=[%s]", sock.id_, name);
    return MOD_OK == module_.accepted_(&sock, name);
}

void
module::connected(mod_handle& sock, const char* name) const AUG_NOTHROW
{
    AUG_CTXDEBUG2(aug_tlx, "connected(): id=[%u], name=[%s]", sock.id_, name);
    module_.connected_(&sock, name);
}

void
module::data(const mod_handle& sock, const char* buf,
             size_t size) const AUG_NOTHROW
{
    AUG_CTXDEBUG2(aug_tlx, "data(): id=[%u]", sock.id_);
    module_.data_(&sock, buf, size);
}

void
module::rdexpire(const mod_handle& sock, unsigned& ms) const AUG_NOTHROW
{
    AUG_CTXDEBUG2(aug_tlx, "rdexpire(): id=[%u], ms=[%u]", sock.id_, ms);
    module_.rdexpire_(&sock, &ms);
}

void
module::wrexpire(const mod_handle& sock, unsigned& ms) const AUG_NOTHROW
{
    AUG_CTXDEBUG2(aug_tlx, "wrexpire(): id=[%u], ms=[%u]", sock.id_, ms);
    module_.wrexpire_(&sock, &ms);
}

void
module::expire(const mod_handle& timer, unsigned& ms) const AUG_NOTHROW
{
    AUG_CTXDEBUG2(aug_tlx, "expire(): id=[%u], ms=[%u]", timer.id_, ms);
    module_.expire_(&timer, &ms);
}

bool
module::authcert(const mod_handle& sock, const char* subject,
                 const char* issuer) const AUG_NOTHROW
{
    AUG_CTXDEBUG2(aug_tlx, "authcert(): id=[%u]", sock.id_);
    return MOD_OK == module_.authcert_(&sock, subject, issuer);
}
