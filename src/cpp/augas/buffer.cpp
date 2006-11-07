/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/buffer.hpp"

#include "augsyspp/unistd.hpp"

using namespace aug;
using namespace augas;

buffer::buffer(size_t size)
    : vec_(size),
      begin_(0),
      end_(0)
{
}

void
buffer::putsome(const void* buf, size_t size)
{
    if (vec_.size() - end_ < size)
        vec_.resize(end_ + size);

    memcpy(&vec_[end_], buf, size);
    end_ += size;
}

bool
buffer::readsome(fdref ref)
{
    char buf[4096];
    size_t size(aug::read(ref, buf, sizeof(buf) - 1));
    if (0 == size)
        return false;

    putsome(buf, size);
    return true;
}

bool
buffer::writesome(fdref ref)
{
    size_t size(end_ - begin_);
    size = aug::write(ref, &vec_[begin_], size);
    if ((begin_ += size) == end_) {
        begin_ = end_ = 0;
        return false;
    }
    return true;
}

bool
buffer::consume(size_t n)
{
    size_t size(end_ - begin_);
    size = AUG_MIN(n, size);
    if ((begin_ += size) == end_) {
        begin_ = end_ = 0;
        return false;
    }
    return true;
}
