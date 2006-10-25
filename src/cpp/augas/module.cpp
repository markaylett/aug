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
    if (!ptr)
        throw local_error(__FILE__, __LINE__, 1, "aug_load() failed");
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
    if (AUGAS_SUCCESS != module_.open_(&s, serv))
        throw local_error(__FILE__, __LINE__, 1,
                          "augas_module::open_() failed");
}

void
module::data(const struct augas_session& s, const char* buf,
             size_t size) const
{
    if (AUGAS_SUCCESS != module_.data_(&s, buf, size))
        throw local_error(__FILE__, __LINE__, 1,
                          "augas_module::data_() failed");
}

void
module::rdexpire(const struct augas_session& s, unsigned& ms) const
{
    if (AUGAS_SUCCESS != module_.rdexpire_(&s, &ms))
        throw local_error(__FILE__, __LINE__, 1,
                          "augas_module::rdexpire_() failed");
}

void
module::wrexpire(const struct augas_session& s, unsigned& ms) const
{
    if (AUGAS_SUCCESS != module_.wrexpire_(&s, &ms))
        throw local_error(__FILE__, __LINE__, 1,
                          "augas_module::wrexpire_() failed");
}

void
module::stop(const struct augas_session& s) const
{
    if (AUGAS_SUCCESS != module_.stop_(&s))
        throw local_error(__FILE__, __LINE__, 1,
                          "augas_module::stop_() failed");
}

void
module::event(int type, void* arg) const
{
    if (AUGAS_SUCCESS != module_.event_(type, arg))
        throw local_error(__FILE__, __LINE__, 1,
                          "augas_module::event_() failed");
}

void
module::expire(void* arg, unsigned id, unsigned* ms) const
{
    if (AUGAS_SUCCESS != module_.expire_(arg, id, ms))
        throw local_error(__FILE__, __LINE__, 1,
                          "augas_module::expire_() failed");
}

void
module::reconf() const
{
    if (AUGAS_SUCCESS != module_.reconf_())
        throw local_error(__FILE__, __LINE__, 1,
                          "augas_module::reconf_() failed");
}
