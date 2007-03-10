/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_BUFFER_HPP
#define AUGAS_BUFFER_HPP

#include "augsyspp/types.hpp"
#include "augnetpp/writer.hpp"

#include <vector>

namespace augas {

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
}

#endif // AUGAS_BUFFER_HPP
