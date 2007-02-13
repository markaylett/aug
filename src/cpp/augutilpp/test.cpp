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

    bool destroyed_ = false;

    void
    destroytest()
    {
        destroyed_ = true;
    }

    long destroyedl_(0);

    void
    destroytest(long l)
    {
        destroyedl_ = l;
    }

    void* destroyedp_(0);

    void
    destroytest(void* p)
    {
        destroyedp_ = p;
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

        destroyvar(var(null, destroytest));
        if (!destroyed_)
            throw logic_error("destroy null test failed");

        destroyvar(var(101, destroytest));
        if (101 != destroyedl_)
            throw logic_error("destroy long test failed");

        destroyvar(var(&argc, destroytest));
        if (&argc != destroyedp_)
            throw logic_error("destroy ptr test failed");

        cout << formatlog(AUG_LOGINFO, "a %s message", "test") << endl;

    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
