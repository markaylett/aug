/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
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
