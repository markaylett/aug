/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_BUFFER_HPP
#define AUGRTPP_BUFFER_HPP

#include "augrtpp/config.hpp"
#include "augsyspp/types.hpp"
#include "augnetpp/writer.hpp"

#include <vector>

#define AUG_VARBUFFER 1

namespace aug {

    struct vecarg {
        std::vector<char> vec_;
        size_t begin_, end_;
        explicit
        vecarg(size_t size)
            : vec_(size),
              begin_(0),
              end_(0)
        {
        }
    };

    class buffer {
        writer writer_;
        vecarg arg_;
        bool usevec_;
        size_t size_;

    public:
        ~buffer() AUG_NOTHROW;

        explicit
        buffer(size_t size = 1024);

        void
        append(const aug_var& var);

        void
        append(const void* buf, size_t size);

        /**
           write, at least some, of the buffered data.

           \return bytes written.
         */

        size_t
        writesome(fdref ref);

        bool
        empty() const
        {
            return writer_.empty();
        }

        /**
           Bytes to be written.

           This value may be inaccurate if the appended var buffers are
           allowed to mutate - they should not.
        */

        size_t
        size() const
        {
            return size_;
        }
    };
}

#endif // AUGRTPP_BUFFER_HPP
