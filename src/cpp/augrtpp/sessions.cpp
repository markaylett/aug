/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/sessions.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include <sstream>

using namespace aug;
using namespace std;

sessions::~sessions() AUG_NOTHROW
{
}

void
sessions::clear()
{
    tmpgroups_.clear();
    groups_.clear();

    // TODO: erase the sessions in reverse order to which they were added.

    sessions_.clear();
}

void
sessions::insert(const string& name, const sessionptr& session,
                 const char* groups)
{
    // The session's start() function may callback into the host.  All state
    // will, therefore, need to be configured prior to calling start().  If
    // start() fails then the state changes will need to be rolled-back.  The
    // tmpgroups_ container is used to store the groups that need to be
    // removed on failure.

    // The implementation is simplified by adding the session name as a group.

    tmpgroups_.insert(make_pair(name, session));

    if (groups) {

        istringstream is(groups);
        string name;
        while (is >> name)
            tmpgroups_.insert(make_pair(name, session));
    }

    // Insert prior to calling open().

    sessions_[name] = session;
    if (!session->start()) {

        // TODO: leave if event posted.

        sessions_.erase(name); // close() will not be called.

    } else
        groups_.insert(tmpgroups_.begin(), tmpgroups_.end());

    tmpgroups_.clear();
}

void
sessions::reconf() const
{
    map<string, sessionptr>::const_iterator it(sessions_.begin()),
        end(sessions_.end());
    for (; it != end; ++it)
        it->second->reconf();
}

sessionptr
sessions::getbyname(const string& name) const
{
    map<string, sessionptr>::const_iterator it(sessions_.find(name));
    if (it == sessions_.end())
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          AUG_MSG("session not found: sname=[%s]"),
                          name.c_str());
    return it->second;
}

void
sessions::getbygroup(vector<sessionptr>& sessions, const string& group) const
{
    pair<groups::const_iterator,
        groups::const_iterator> its(groups_.equal_range(group));

    for (; its.first != its.second; ++its.first)
        sessions.push_back(its.first->second);

    // Include any groups in the temporary container.

    its = tmpgroups_.equal_range(group);

    for (; its.first != its.second; ++its.first)
        sessions.push_back(its.first->second);
}
