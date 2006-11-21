/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsyspp/base.hpp"
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
    verifyproxy(bool result)
    {
        which_ = BOOL;
        return verify(result);
    }

    template <typename T>
    T
    verifyproxy(T result)
    {
        which_ = INT;
        return verify(result);
    }

    template <typename T>
    T*
    verifyproxy(T* result)
    {
        which_ = PTR;
        return verify(result);
    }

    void
    throwtest()
    {
        try {
            throw basic_error<AUG_SRCUSER>(__FILE__, __LINE__, 1, "test");
        } catch (const aug::errinfo_error& e) {
            if (0 != strcmp("test", errdesc(e)))
                throw error("invalid error description");
        }
    }
}

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    aug_atexitinit(&errinfo);

    try {

        bool b(true);
        b = verifyproxy(b);
        if (BOOL != which_)
            throw error("failed to call bool template");
        if (!b)
            throw error("returned bool differs from original");

        int i(101);
        i = verifyproxy(i);
        if (INT != which_)
            throw error("failed to call int template");
        if (101 != i)
            throw error("returned int differs from original");

        int* p = &i;
        p = verifyproxy(p);
        if (PTR != which_)
            throw error("failed to call pointer template");
        if (&i != p)
            throw error("returned pointer differs from original");

        const int* cp = &i;
        cp = verifyproxy(cp);
        if (PTR != which_)
            throw error("failed to call const pointer template");
        if (&i != cp)
            throw error("returned const pointer differs from original");

        throwtest();
        return 0;

    } AUG_PERRINFOCATCH;
    return 1;
}

