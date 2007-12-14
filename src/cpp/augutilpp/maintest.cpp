/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include <cstring>
#include <iostream>
#include <string>

using namespace std;

#if 0
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
#endif

int
main(int argc, char* argv[])
{
    try {
#if 0
        string* s(new string());
        aug_var v;
        bindvar<test>(v, null);
        bindvar<test>(v, *s);
        s->assign("test");
        memcmp(varbuf<char>(v), "test", 4);
        destroyvar(v);
#endif
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
