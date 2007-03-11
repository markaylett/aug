/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_BUFFER_HPP
#define AUGAS_BUFFER_HPP

#include "augsyspp/types.hpp"
#include "augnetpp/writer.hpp"

#include <vector>

namespace augas {

#if !AUGAS_VARBUFFER
    class buffer {
        aug::writer writer_;
    public:
        std::vector<char> vec_;
        size_t begin_, end_;

        explicit
        buffer(size_t size = 4096);

        void
        append(const aug_var& var);

        void
        append(const void* buf, size_t size);

        bool
        writesome(aug::fdref ref);

        bool
        empty() const
        {
            return begin_ == end_;
        }
    };
#else // AUGAS_VARBUFFER
    class buffer {
        aug::writer writer_;
    public:
        void
        append(const aug_var& var)
        {
            appendbuf(writer_, var);
        }
        void
        append(const void* buf, size_t size);

        bool
        writesome(aug::fdref ref);

        bool
        empty() const
        {
            return writer_.empty();
        }
    };
#endif // AUGAS_VARBUFFER
}

#endif // AUGAS_BUFFER_HPP
