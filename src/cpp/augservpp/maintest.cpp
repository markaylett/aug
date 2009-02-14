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
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

    typedef unsigned long nodeid;
    typedef map<string, nodeid> nodenames;
    typedef vector<nodenames> pathnames;
    typedef vector<nodeid> path;
    typedef logic_error error;

    nodeid
    nextid()
    {
        static nodeid i(1);
        return i++;
    }

    bool
    match(const path& filter, const path& path)
    {
        size_t i(0), n(filter.size());
        if (n != path.size())
            throw error("invalid path nodes");

        for (; i < n; ++i) {
            nodeid id(filter[i]);
            if (0 != id && path[i] != id)
                return false;
        }
        return true;
    }
}

int
main(int argc, char* argv[])
{
    try {

        path a, b;

        a.push_back(0);
        a.push_back(0);
        a.push_back(4);

        b.push_back(1);
        b.push_back(2);
        b.push_back(3);

        cout << match(a, b) << endl;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }

    return 0;
}
