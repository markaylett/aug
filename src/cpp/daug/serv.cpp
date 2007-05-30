/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define DAUG_BUILD
#include "daug/serv.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys.h"

#include <stack>

using namespace aug;
using namespace augrt;
using namespace std;

namespace {
    stack<const augrt_serv*> stack_;
    struct scoped_frame {
        ~scoped_frame() AUG_NOTHROW
        {
            stack_.pop();
        }
        explicit
        scoped_frame(const augrt_serv* serv)
        {
            stack_.push(serv);
        }
    };
}

augrt_serv&
serv::do_get() AUG_NOTHROW
{
    return serv_;
}

const augrt_serv&
serv::do_get() const AUG_NOTHROW
{
    return serv_;
}

bool
serv::do_active() const AUG_NOTHROW
{
    return active_;
}

bool
serv::do_start() AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    active_ = true; // Functions may be called during initialisation.
    return active_ = module_->start(serv_);
}

void
serv::do_reconf() const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->reconf();
}

void
serv::do_event(const char* from, const char* type, const void* user,
               size_t size) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->event(from, type, user, size);
}

void
serv::do_closed(const augrt_object& sock) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->closed(sock);
}

void
serv::do_teardown(const augrt_object& sock) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->teardown(sock);
}

bool
serv::do_accepted(augrt_object& sock, const char* addr,
                  unsigned short port) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    return module_->accepted(sock, addr, port);
}

void
serv::do_connected(augrt_object& sock, const char* addr,
                   unsigned short port) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->connected(sock, addr, port);
}

void
serv::do_data(const augrt_object& sock, const char* buf,
              size_t size) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->data(sock, buf, size);
}

void
serv::do_rdexpire(const augrt_object& sock, unsigned& ms) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->rdexpire(sock, ms);
}

void
serv::do_wrexpire(const augrt_object& sock, unsigned& ms) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->wrexpire(sock, ms);
}

void
serv::do_expire(const augrt_object& timer, unsigned& ms) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->expire(timer, ms);
}

bool
serv::do_authcert(const augrt_object& sock, const char* subject,
                  const char* issuer) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    return module_->authcert(sock, subject, issuer);
}

serv::~serv() AUG_NOTHROW
{
    if (active_) {
        scoped_frame frame(&serv_);
        module_->stop(); // AUG_NOTHROW
    }
}

serv::serv(const moduleptr& module, const char* name)
    : module_(module),
      active_(false)
{
    aug_strlcpy(serv_.name_, name, sizeof(serv_.name_));
    serv_.user_ = 0;
}

const augrt_serv*
augrt::getserv()
{
    return stack_.empty() ? 0 : stack_.top();
}
