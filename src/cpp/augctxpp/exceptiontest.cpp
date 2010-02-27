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
        autotlx();

        bool b(true);
        b = verifyproxy(b);
        if (BOOL_ != which_)
            throw error("failed to call bool template");
        if (!b)
            throw error("returned bool differs from original");

        aug_result r(101);
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

