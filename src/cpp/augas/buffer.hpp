/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_BUFFER_HPP
#define AUGAS_BUFFER_HPP

#include "augsyspp/types.hpp"

#include <vector>

namespace augas {

    class buffer {
        std::vector<char> vec_;
        size_t begin_, end_;
    public:
        explicit
        buffer(size_t size = 4096);

        void
        putsome(const void* buf, size_t size);

        bool
        writesome(aug::fdref ref);

        bool
        consume(size_t n);

        bool
        empty() const
        {
            return begin_ == end_;
        }
    };
}

#endif // AUGAS_BUFFER_HPP
