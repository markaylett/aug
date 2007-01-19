/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augutilpp/log.hpp"
#include "augutilpp/var.hpp"

#include <iostream>
#include <stdexcept>

using namespace aug;
using namespace std;

namespace {

    bool freed_ = false;

    void
    freetest()
    {
        freed_ = true;
    }

    long freedl_(0);

    void
    freetest(long l)
    {
        freedl_ = l;
    }

    void* freedp_(0);

    void
    freetest(void* p)
    {
        freedp_ = p;
    }
}

int
main(int argc, char* argv[])
{
    try {

        var v(null);

        if (AUG_VTNULL != type(v) || 0 != getvar<long>(v)
            || 0 != getvar<void*>(v))
            throw logic_error("null constructor failed");

        setvar(v, 101);

        if (AUG_VTLONG != type(v) || 101 != getvar<long>(v)
            || 0 != getvar<void*>(v))
            throw logic_error("long assignment failed");

        setvar(v, &argc);

        if (AUG_VTPTR != type(v) || 0 != getvar<long>(v)
            || &argc != getvar<void*>(v))
            throw logic_error("pointer assignment failed");

        v = null;

        if (AUG_VTNULL != type(v) || 0 != getvar<long>(v)
            || 0 != getvar<void*>(v))
            throw logic_error("null assignment failed");

        if (v != null || null != v)
            throw logic_error("equality test failed");

        setvar(v, 100);

        var w(100);
        if (v != w)
            throw logic_error("equality test failed");

        setvar(w, 200);
        if (v == w)
            throw logic_error("equality test failed");

        freevar(var(null, freetest));
        if (!freed_)
            throw logic_error("free null test failed");

        freevar(var(101, freetest));
        if (101 != freedl_)
            throw logic_error("free long test failed");

        freevar(var(&argc, freetest));
        if (&argc != freedp_)
            throw logic_error("free ptr test failed");

        cout << formatlog(AUG_LOGINFO, "a %s message", "test") << endl;

    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
