/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augutilpp/var.hpp"

#include <iostream>
#include <stdexcept>

using namespace aug;
using namespace std;

int
main(int argc, char* argv[])
{
    try {

        var v(null);

        if (AUG_VTNULL != v.type() || 0 != getvar<long>(v)
            || 0 != getvar<void*>(v))
            throw logic_error("null constructor failed");

        v = 101;

        if (AUG_VTLONG != v.type() || 101 != getvar<long>(v)
            || 0 != getvar<void*>(v))
            throw logic_error("long assignment failed");

        v = &argc;

        if (AUG_VTPTR != v.type() || 0 != getvar<long>(v)
            || &argc != getvar<void*>(v))
            throw logic_error("pointer assignment failed");

        v = null;

        if (AUG_VTNULL != v.type() || 0 != getvar<long>(v)
            || 0 != getvar<void*>(v))
            throw logic_error("null assignment failed");

        if (v != null || null != v)
            throw logic_error("equality test failed");

        v = 100;
        var w(100);
        if (v != w)
            throw logic_error("equality test failed");

        w = 200;
        if (v == w)
            throw logic_error("equality test failed");

    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
