/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/servs.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include <sstream>

using namespace aug;
using namespace std;

AUGRTPP_API
servs::~servs() AUG_NOTHROW
{
}

AUGRTPP_API void
servs::clear()
{
    tmpgroups_.clear();
    groups_.clear();

    // TODO: erase the services in reverse order to which they were added.

    servs_.clear();
}

AUGRTPP_API void
servs::insert(const string& name, const servptr& serv, const char* groups)
{
    // The service's start() function may callback into the host.  All state
    // will, therefore, need to be configured prior to calling start().  If
    // start() fails then the state changes will need to be rolled-back.  The
    // tmpgroups_ container is used to store the groups that need to be
    // removed on failure.

    // The implementation is simplified by adding the service name as a group.

    tmpgroups_.insert(make_pair(name, serv));

    if (groups) {

        istringstream is(groups);
        string name;
        while (is >> name)
            tmpgroups_.insert(make_pair(name, serv));
    }

    // Insert prior to calling open().

    servs_[name] = serv;
    if (!serv->start()) {

        // TODO: leave if event posted.

        servs_.erase(name); // close() will not be called.

    } else
        groups_.insert(tmpgroups_.begin(), tmpgroups_.end());

    tmpgroups_.clear();
}

AUGRTPP_API void
servs::reconf() const
{
    map<string, servptr>::const_iterator it(servs_.begin()),
        end(servs_.end());
    for (; it != end; ++it)
        it->second->reconf();
}

AUGRTPP_API servptr
servs::getbyname(const string& name) const
{
    map<string, servptr>::const_iterator it(servs_.find(name));
    if (it == servs_.end())
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          AUG_MSG("service not found: sname=[%s]"),
                          name.c_str());
    return it->second;
}

AUGRTPP_API void
servs::getbygroup(vector<servptr>& servs, const string& group) const
{
    pair<groups::const_iterator,
        groups::const_iterator> its(groups_.equal_range(group));

    for (; its.first != its.second; ++its.first)
        servs.push_back(its.first->second);

    // Include any groups in the temporary container.

    its = tmpgroups_.equal_range(group);

    for (; its.first != its.second; ++its.first)
        servs.push_back(its.first->second);
}
