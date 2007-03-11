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

#if !AUGAS_VARBUFFER

namespace {

    struct vartype : basic_vartype<buffer> {
        static const void*
        buf(arg_type& arg, size_t& size)
        {
            size = arg.end_ - arg.begin_;
            return &arg.vec_[arg.begin_];
        }
        static const void*
        buf(arg_type& arg)
        {
            return &arg.vec_[arg.begin_];
        }
    };
}

buffer::buffer(size_t size)
    : vec_(size),
      begin_(0),
      end_(0)
{
}

void
buffer::append(const aug_var& var)
{
    size_t size;
    const void* buf(varbuf(var, size));
    append(buf, size);
}

void
buffer::append(const void* buf, size_t size)
{
    if (empty()) {
        aug_var v;
        appendbuf(writer_, bindvar<vartype>(v, *this));
    }

    if (vec_.size() - end_ < size)
        vec_.resize(end_ + size);

    memcpy(&vec_[end_], buf, size);
    end_ += size;
}

bool
buffer::writesome(fdref ref)
{
    size_t size(aug::writesome(writer_, ref));
    if ((begin_ += size) == end_) {
        begin_ = end_ = 0;
        return true;
    }
    return false;
}

#else // AUGAS_VARBUFFER

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

#endif // AUGAS_VARBUFFER
