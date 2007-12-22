/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augutilpp/object.hpp"

#include "augob/blob.h"

#include <iostream>
#include <stdexcept>

using namespace aub;
using namespace aug;
using namespace std;

namespace {
    typedef logic_error error;

    void
    test(obref<aug_blob> blob, const string& s)
    {
        size_t size;
        const void* data(blobdata(blob, &size));

        if (size != s.size())
            throw error("size mismatch");

        if (string(static_cast<const char*>(data), size) != s)
            throw error("data mismatch");
    }
}

int
main(int argc, char* argv[])
{
    try {

        const string s("some test data");

        smartob<aug_blob> smart(basic_blob<stringob>::create(s));
        test(smart, s);

        if (null == smart)
            throw error("bad null equality");

        smart = null;
        if (null != smart)
            throw error("bad null inequality");

        scoped_blob<stringob> scoped(s);
        test(scoped, s);

    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
