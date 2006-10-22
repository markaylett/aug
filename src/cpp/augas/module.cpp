/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augas/module.hpp"

#include "augas/utility.hpp"

#include "augsys/log.h"

#include <exception>

using namespace aug;
using namespace augas;
using namespace std;

module::~module() AUG_NOTHROW
{
    try {
        unloadfn_();
    } catch (const exception& e) {
        aug_error("unexpected exception: %s", e.what());
    } catch (...) {
        aug_error("unexpected exception");
    }
}

module::module(const char* path, const struct augas_service& service)
    : lib_(path)
{
    augas_loadfn loadfn(dlsym<augas_loadfn>(lib_, "augas_load"));
    unloadfn_ = dlsym<augas_unloadfn>(lib_, "augas_unload");

    const struct augas_module* ptr(loadfn(&service));
    if (!ptr) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EUSER,
                       "module error");
        throwerrinfo("augas_load() failed");
    }
    setdefaults(module_, *loadfn(&service));
}

void
module::close(const struct augas_session& s) const
{
    module_.close_(&s);
}

void
module::open(struct augas_session& s, const char* serv) const
{
    if (AUGAS_SUCCESS != module_.open_(&s, serv)) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EUSER,
                       "module error");
        throwerrinfo("augas_module::open_() failed");
    }
}

int
module::data(const struct augas_session& s, const char* buf,
             size_t size) const
{
    return module_.data_(&s, buf, size);
}

int
module::rdexpire(const struct augas_session& s, unsigned& ms) const
{
    return module_.rdexpire_(&s, &ms);
}

int
module::wrexpire(const struct augas_session& s, unsigned& ms) const
{
    return module_.wrexpire_(&s, &ms);
}

int
module::stop(const struct augas_session& s) const
{
    return module_.stop_(&s);
}

int
module::event(int type, void* arg) const
{
    return module_.event_(type, arg);
}

int
module::expire(void* arg, unsigned id, unsigned* ms) const
{
    return module_.expire_(arg, id, ms);
}

int
module::reconf() const
{
    return module_.reconf_();
}
