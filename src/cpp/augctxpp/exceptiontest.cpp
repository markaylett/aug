/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augctxpp/base.hpp"
#include "augctxpp/exception.hpp"

#include <iostream>
#include <stdexcept>

using namespace aug;
using namespace std;

enum which {
    NONE_ = 0,
    BOOL_,
    RESULT_,
    PTR_
};

namespace {

    typedef logic_error error;

    which which_ = NONE_;

    bool
    verifyproxy(bool result)
    {
        which_ = BOOL_;
        return verify(result);
    }

    template <typename T>
    T
    verifyproxy(T result)
    {
        which_ = RESULT_;
        return verify(result);
    }

    template <typename T>
    T*
    verifyproxy(T* result)
    {
        which_ = PTR_;
        return verify(result);
    }

    const char*
    errorsrc()
    {
        return "src";
    }

    void
    throwtest()
    {
        try {
            throw basic_error<errorsrc>(__FILE__, __LINE__, 1, "test");
        } catch (const aug::errinfo_error& e) {
            if (0 != strcmp("test", errdesc(e)))
                throw error("invalid error description");
        }
    }

    void
    rethrowtest()
    {
        try {
            try {
                throw basic_error<errorsrc>(__FILE__, __LINE__, 1, "test");
            } catch (...) {
                throw;
            }
        } catch (const aug::errinfo_error& e) {
            if (0 != strcmp("test", errdesc(e)))
                throw error("invalid error description");
        }
    }
}

int
main(int argc, char* argv[])
{
    try {
        autobasictlx();

        bool b(true);
        b = verifyproxy(b);
        if (BOOL_ != which_)
            throw error("failed to call bool template");
        if (!b)
            throw error("returned bool differs from original");

        aug_result r(AUG_MKRESULT(101));
        r = verifyproxy(r);
        if (RESULT_ != which_)
            throw error("failed to call result template");
        if (101 != AUG_RESULT(r))
            throw error("returned result differs from original");

        int i(101);
        int* p = &i;
        p = verifyproxy(p);
        if (PTR_ != which_)
            throw error("failed to call pointer template");
        if (&i != p)
            throw error("returned pointer differs from original");

        const int* cp = &i;
        cp = verifyproxy(cp);
        if (PTR_ != which_)
            throw error("failed to call const pointer template");
        if (&i != cp)
            throw error("returned const pointer differs from original");

        throwtest();
        rethrowtest();
        return 0;

    } AUG_PERRINFOCATCH;
    return 1;
}

