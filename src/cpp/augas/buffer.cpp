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

    struct vectype : basic_vartype<vecarg> {
        static void
        destroy(arg_type* arg)
        {
            arg->begin_ = arg->end_ = 0;
        }
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

    struct copyarg {
        size_t size_;
        char buf_[1];
    };

    struct copytype : basic_vartype<copyarg> {
        static void
        destroy(arg_type* arg)
        {
            free(arg);
        }
        static const void*
        buf(arg_type& arg, size_t& size)
        {
            size = arg.size_;
            return arg.buf_;
        }
        static const void*
        buf(arg_type& arg)
        {
            return arg.buf_;
        }
    };
}

buffer::buffer(size_t size)
    : arg_(size),
      usevec_(true)
{
}

void
buffer::append(const aug_var& var)
{
    appendbuf(writer_, var);
    usevec_ = false;
}

void
buffer::append(const void* buf, size_t size)
{
    if (usevec_) {

        if (empty()) {
            aug_var var;
            appendbuf(writer_, bindvar<vectype>(var, arg_));
        }

        // Grow buffer if needed.

        if (arg_.vec_.size() - arg_.end_ < size)
            arg_.vec_.resize(arg_.end_ + size);

        memcpy(&arg_.vec_[arg_.end_], buf, size);
        arg_.end_ += size;

    } else {

        copyarg* arg(static_cast<copyarg*>(malloc(sizeof(copyarg)
                                                  + size - 1)));
        arg->size_ = size;
        memcpy(arg->buf_, buf, size);

        aug_var var;
        appendbuf(writer_, bindvar<copytype>(var, *arg));
    }
}

bool
buffer::writesome(fdref ref)
{
    aug::writesome(writer_, ref);
    if (writer_.empty()) {
        usevec_ = true;
        return true;
    }
    return false;
}
