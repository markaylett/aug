/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
        if (null != p2)
            throw error("failed null assignment");
        if (!free_)
            throw error("failed to destroy smartptr");

    } catch (const exception& e) {
        cerr << "error: " << e.what() << endl;
    }
    return 0;
}

