/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define AUGASPP_BUILD
#include "augaspp/sessions.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctxpp/exception.hpp"

#include <sstream>

using namespace aug;
using namespace std;

sessions::~sessions() AUG_NOTHROW
{
}

void
sessions::clear()
{
    topics_.clear();

    // TODO: erase the sessions in reverse order to which they were added.

    sessions_.clear();
}

void
sessions::insert(const string& name, const sessionptr& session,
                 const char* topics)
{
    // A session's start() function may callback into the host.  All
    // initialisation will, therefore, need to be conducted prior to calling
    // start().  If start() fails, then any state changes will be rolled-back.
    // The tmptopics_ container is used to store the topics that need to be
    // unwound on failure.

    struct scoped_topics {
        multimap<string, sessionptr>& tmp_;
        ~scoped_topics()
        {
            tmp_.clear();
        }
        scoped_topics(multimap<string, sessionptr>& tmp)
            : tmp_(tmp)
        {
        }
    } tmp(tmptopics_);

    // The implementation is simplified by adding the session name as a topic.

    tmptopics_.insert(make_pair(name, session));

    if (topics) {

        istringstream is(topics);
        string name;
        while (is >> name)
            tmptopics_.insert(make_pair(name, session));
    }

    // Insert prior to calling open().

    sessions_[name] = session;
    if (!session->start()) {

        AUG_CTXDEBUG2(aug_tlx, "session not started: [%s]", name.c_str());

        // TODO: keep this session in an inactive state rather then remove it.
        // This is required as blobs may assume the existence of their
        // originating session.

        sessions_.erase(name); // stop() should not be called.

    } else
        topics_.insert(tmptopics_.begin(), tmptopics_.end());
}

void
sessions::reconf() const
{
    map<string, sessionptr>::const_iterator it(sessions_.begin()),
        end(sessions_.end());
    for (; it != end; ++it)
        it->second->reconf();
}

void
sessions::getsessions(std::vector<sessionptr>& sessions) const
{
    map<string, sessionptr>::const_iterator it(sessions_.begin()),
        end(sessions_.end());
    for (; it != end; ++it)
        sessions.push_back(it->second);
}

sessionptr
sessions::getbyname(const string& name) const
{
    map<string, sessionptr>::const_iterator it(sessions_.find(name));
    if (it == sessions_.end())
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        AUG_MSG("session not found: sname=[%s]"),
                        name.c_str());
    return it->second;
}

void
sessions::getbytopic(vector<sessionptr>& sessions, const string& topic) const
{
    pair<topics::const_iterator,
        topics::const_iterator> its(topics_.equal_range(topic));

    for (; its.first != its.second; ++its.first)
        sessions.push_back(its.first->second);

    // Include any topics in the temporary container.

    its = tmptopics_.equal_range(topic);

    for (; its.first != its.second; ++its.first)
        sessions.push_back(its.first->second);
}
