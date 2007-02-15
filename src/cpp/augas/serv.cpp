/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/serv.hpp"

#include "augsys.h"

using namespace aug;
using namespace augas;
using namespace std;

serv::~serv() AUG_NOTHROW
{
    if (active_)
        module_->destroy(serv_); // AUG_NOTHROW
}

serv::serv(const moduleptr& module, const char* name)
    : module_(module),
      active_(false)
{
    aug_strlcpy(serv_.name_, name, sizeof(serv_.name_));
    serv_.user_ = 0;
}
