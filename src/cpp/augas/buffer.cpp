/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/buffer.hpp"

#include "augutilpp/var.hpp"

#include <string>

using namespace aug;
using namespace augas;
using namespace std;

namespace {

    struct vartype : basic_vartype<string> {
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

void
buffer::append(const void* buf, size_t size)
{
    string* s(new string(static_cast<const char*>(buf), size));
    aug_var v;
    appendbuf(writer_, bindvar<vartype>(v, *s));
}

bool
buffer::writesome(fdref ref)
{
    aug::writesome(writer_, ref);
    return writer_.empty();
}
