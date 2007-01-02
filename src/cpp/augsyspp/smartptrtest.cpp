/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsyspp/smartptr.hpp"

#include <iostream>
#include <stdexcept>

using namespace aug;
using namespace std;

namespace {

    typedef logic_error error;

    bool free_ = true;

    struct test {
        int i_;
        ~test()
        {
            free_ = true;
        }
        explicit
        test(int i)
            : i_(i)
        {
            free_ = false;
        }
    };

    typedef smartptr<test> testptr;
}

int
main(int argc, char* argv[])
{
    try {
        {
            testptr p1(new test(101));
            if (free_ || 101 != p1->i_)
                throw error("failed to create smartptr");
        }
        if (!free_)
            throw error("failed to destroy smartptr");
        testptr p2 = null;
        if (null != p2)
            throw error("comparison to null failed");
        p2 = testptr(new test(202));
        if (free_ || 202 != p2->i_)
            throw error("failed to assign smartptr");
        p2 = null;
        if (!free_)
            throw error("failed to destroy smartptr");

    } catch (const exception& e) {
        cerr << "error: " << e.what() << endl;
    }
    return 0;
}

