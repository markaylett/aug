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
        termfn_();
    } catch (const exception& e) {
        aug_error("std::exception: %s", e.what());
    } catch (...) {
        aug_error("unknown exception");
    }
}

module::module(const string& name, const char* path,
               const struct augas_host& host)
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
    setdefaults(module_, *ptr);
}

void
module::closesess(const augas_sess& sess) const
{
    AUG_DEBUG2("closesess(): sname=[%s]", sess.name_);
    module_.closesess_(&sess);
}

void
module::opensess(augas_sess& sess) const
{
    AUG_DEBUG2("opensess(): sname=[%s]", sess.name_);
    if (AUGAS_OK != module_.opensess_(&sess))
        throw error(__FILE__, __LINE__, EMODCALL,
                    "augas_module::opensess_() failed");
}

void
module::event(const augas_sess& sess, int type, void* user) const
{
    AUG_DEBUG2("event(): sname=[%s]", sess.name_);
    if (AUGAS_OK != module_.event_(&sess, type, user))
        throw error(__FILE__, __LINE__, EMODCALL,
                    "augas_module::event_() failed");
}

void
module::expire(const augas_sess& sess, int tid, void* user,
               unsigned& ms) const
{
    AUG_DEBUG2("expire(): sname=[%s]", sess.name_);
    if (AUGAS_OK != module_.expire_(&sess, tid, user, &ms))
        throw error(__FILE__, __LINE__, EMODCALL,
                    "augas_module::expire_() failed");
}

void
module::reconf(const augas_sess& sess) const
{
    AUG_DEBUG2("reconf(): sname=[%s]", sess.name_);
    if (AUGAS_OK != module_.reconf_(&sess))
        throw error(__FILE__, __LINE__, EMODCALL,
                    "augas_module::reconf_() failed");
}

void
module::close(const augas_file& file) const
{
    AUG_DEBUG2("close(): sname=[%s], id=[%d]", file.sess_->name_, file.id_);
    module_.close_(&file);
}

void
module::accept(augas_file& file, const char* addr, unsigned short port) const
{
    AUG_DEBUG2("accept(): sname=[%s], id=[%d]", file.sess_->name_, file.id_);
    if (AUGAS_OK != module_.accept_(&file, addr, port))
        throw error(__FILE__, __LINE__, EMODCALL,
                    "augas_module::accept_() failed");
}

void
module::connect(augas_file& file, const char* addr, unsigned short port) const
{
    AUG_DEBUG2("connect(): sname=[%s], id=[%d]", file.sess_->name_, file.id_);
    if (AUGAS_OK != module_.connect_(&file, addr, port))
        throw error(__FILE__, __LINE__, EMODCALL,
                    "augas_module::connect_() failed");
}

void
module::data(const augas_file& file, const char* buf, size_t size) const
{
    AUG_DEBUG2("data(): sname=[%s], id=[%d]", file.sess_->name_, file.id_);
    if (AUGAS_OK != module_.data_(&file, buf, size))
        throw error(__FILE__, __LINE__, EMODCALL,
                    "augas_module::data_() failed");
}

void
module::rdexpire(const augas_file& file, unsigned& ms) const
{
    AUG_DEBUG2("rdexpire(): sname=[%s], id=[%d]", file.sess_->name_,
               file.id_);
    if (AUGAS_OK != module_.rdexpire_(&file, &ms))
        throw error(__FILE__, __LINE__, EMODCALL,
                    "augas_module::rdexpire_() failed");
}

void
module::wrexpire(const augas_file& file, unsigned& ms) const
{
    AUG_DEBUG2("wrexpire(): sname=[%s], id=[%d]", file.sess_->name_,
               file.id_);
    if (AUGAS_OK != module_.wrexpire_(&file, &ms))
        throw error(__FILE__, __LINE__, EMODCALL,
                    "augas_module::wrexpire_() failed");
}

void
module::teardown(const augas_file& file) const
{
    AUG_DEBUG2("teardown(): sname=[%s], id=[%d]", file.sess_->name_,
               file.id_);
    if (AUGAS_OK != module_.teardown_(&file))
        throw error(__FILE__, __LINE__, EMODCALL,
                    "augas_module::teardown_() failed");
}
