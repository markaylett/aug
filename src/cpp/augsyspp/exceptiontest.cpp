/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsyspp/exception.hpp"

#include <iostream>
#include <stdexcept>

using namespace aug;
using namespace std;

namespace {

    typedef logic_error error;

    enum which {
        NONE = 0,
        BOOL,
        INT,
        PTR
    } which_ = NONE;

    bool
    verify_proxy(bool result)
    {
        which_ = BOOL;
        return verify(result);
    }

    template <typename T>
    T
    verify_proxy(T result)
    {
        which_ = INT;
        return verify(result);
    }

    template <typename T>
    T*
    verify_proxy(T* result)
    {
        which_ = PTR;
        return verify(result);
    }
}

int
main(int argc, char* argv[])
{
    try {
        bool b(true);
        b = verify_proxy(b);
        if (BOOL != which_)
            throw error("failed to call bool template");
        if (!b)
            throw error("returned bool differs from original");

        int i(101);
        i = verify_proxy(i);
        if (INT != which_)
            throw error("failed to call int template");
        if (101 != i)
            throw error("returned int differs from original");

        int* p = &i;
        p = verify_proxy(p);
        if (PTR != which_)
            throw error("failed to call pointer template");
        if (&i != p)
            throw error("returned pointer differs from original");

        const int* cp = &i;
        cp = verify_proxy(cp);
        if (PTR != which_)
            throw error("failed to call const pointer template");
        if (&i != cp)
            throw error("returned const pointer differs from original");

    } catch (const exception& e) {
        cerr << "error: " << e.what() << endl;
    }
    return 0;
}

