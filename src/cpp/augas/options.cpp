/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augas/options.hpp"

using namespace augas;
using namespace std;

void
options::do_callback(const char* name, const char* value)
{
    options_[name] = value;
}

options::~options() AUG_NOTHROW
{
}

void
options::read(const char* path)
{
    options_.clear();
    readconf(path, *this);
}

void
options::set(const string& name, const string& value)
{
    options_[name] = value;
}

const char*
options::get(const string& name, const char* def) const
{
    map<string, string>::const_iterator it(options_.find(name));
    if (options_.find(name) == options_.end())
        return def;
    return it->second.c_str();
}
