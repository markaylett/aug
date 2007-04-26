/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/serv.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys.h"

#include <stack>

using namespace aug;
using namespace augas;
using namespace std;

namespace {
    stack<const augas_serv*> stack_;
    struct scoped_frame {
        ~scoped_frame() AUG_NOTHROW
        {
            stack_.pop();
        }
        explicit
        scoped_frame(const augas_serv* serv)
        {
            stack_.push(serv);
        }
    };
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

bool
serv::start() AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    active_ = true; // Functions may be called during initialisation.
    return active_ = module_->start(serv_);
}

void
serv::reconf() const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->reconf();
}

void
serv::event(const char* from, const char* type, const void* user,
            size_t size) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->event(from, type, user, size);
}

void
serv::closed(const augas_object& sock) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->closed(sock);
}

void
serv::teardown(const augas_object& sock) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->teardown(sock);
}

bool
serv::accept(augas_object& sock, const char* addr,
             unsigned short port) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    return module_->accept(sock, addr, port);
}

void
serv::connected(augas_object& sock, const char* addr,
                unsigned short port) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->connected(sock, addr, port);
}

void
serv::data(const augas_object& sock, const char* buf,
           size_t size) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->data(sock, buf, size);
}

void
serv::rdexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->rdexpire(sock, ms);
}

void
serv::wrexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->wrexpire(sock, ms);
}

void
serv::expire(const augas_object& timer, unsigned& ms) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    module_->expire(timer, ms);
}

bool
serv::authcert(const augas_object& sock, const char* subject,
               const char* issuer) const AUG_NOTHROW
{
    scoped_frame frame(&serv_);
    return module_->authcert(sock, subject, issuer);
}

const augas_serv*
augas::getserv()
{
    return stack_.empty() ? 0 : stack_.top();
}
