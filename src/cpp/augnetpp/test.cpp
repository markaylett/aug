/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augutilpp/var.hpp"

#include <cstring>
#include <iostream>
#include <string>

using namespace aug;
using namespace std;

namespace {

    struct test : basic_vartype<string> {
        static void
        destroy(arg_type* arg)
        {
            delete arg;
        }
        static const void*
        buf(arg_type& arg, size_t& size)
        {
            size = arg.size();
            return arg.data();
        }
        static const void*
        buf(arg_type& arg)
        {
            return arg.data();
        }
    };
}

int
main(int argc, char* argv[])
{
    try {
        string* s(new string());
        aug_var v;
        bindvar<test>(v, null);
        bindvar<test>(v, *s);
        s->assign("test");
        memcmp(varbuf<char>(v), "test", 4);
        destroyvar(v);
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
#if 0
/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include <iostream>

using namespace std;

int
main(int argc, char* argv[])
{
    try {

    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
#endif
