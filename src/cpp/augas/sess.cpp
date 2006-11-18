/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/sess.hpp"

#include "augsys.h"

using namespace aug;
using namespace augas;
using namespace std;

sess::~sess() AUG_NOTHROW
{
    try {
        if (open_)
            module_->closesess(sess_);
    } AUG_PERRINFOCATCH;
}

sess::sess(const moduleptr& module, const char* name)
    : module_(module),
      open_(false)
{
    aug_strlcpy(sess_.name_, name, sizeof(sess_.name_));
    sess_.user_ = 0;
}

void
sess::open()
{
    if (!open_) {
        module_->opensess(sess_);
        open_ = true;
    }
}
