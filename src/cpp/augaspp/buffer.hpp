/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_BUFFER_HPP
#define AUGRTPP_BUFFER_HPP

#include "augaspp/config.hpp"
#include "augsyspp/types.hpp"
#include "augnetpp/writer.hpp"

#include <vector>

#define AUG_VARBUFFER 1

namespace aug {

    namespace detail {
        class vecblob {
            blob<vecblob> blob_;
            std::vector<char> vec_;
            size_t size_;
            unsigned refs_;
        public:
            explicit
            vecblob(size_t size)
                : vec_(size),
                  size_(0),
                  refs_(1)
            {
                blob_.reset(this);
            }
            aug::smartob<aug_object>
            cast_(const char* id) AUG_NOTHROW
            {
                if (aug::equalid<aug_object>(id)
                    || aug::equalid<aug_blob>(id))
                    return aug::object_retain<aug_object>(blob_);
                return null;
            }
            int
            retain_() AUG_NOTHROW
            {
                ++refs_;
                return 0;
            }
            int
            release_() AUG_NOTHROW
            {
                // When only one ref remains, writing must have finished and
                // vector can be reset.

                if (1 == --refs_)
                    size_ = 0;
                return 0;
            }
            const void*
            blobdata_(size_t* size) AUG_NOTHROW
            {
                if (size)
                    *size = size_;
                return &vec_[0];
            }
            size_t
            blobsize_() AUG_NOTHROW
            {
                return size_;
            }
            void
            append(const void* buf, size_t len)
            {
                // Grow buffer if needed.

                if (vec_.size() - size_ < len)
                    vec_.resize(size_ + len);

                memcpy(&vec_[size_], buf, len);
                size_ += len;
            }
            aug_blob*
            get()
            {
                return blob_.get();
            }
            operator blobref()
            {
                return blob_;
            }
        };
    }

    class buffer {
        writer writer_;
        detail::vecblob blob_;
        bool reuse_;
        size_t size_;

    public:
        ~buffer() AUG_NOTHROW;

        explicit
        buffer(size_t size = 1024);

        void
        append(blobref ref);

        void
        append(const void* buf, size_t size);

        /**
         * write, at least some, of the buffered data.
         *
         * @return Bytes written.
         */

        size_t
        writesome(fdref ref);

        bool
        empty() const
        {
            return writer_.empty();
        }

        /**
         * Bytes to be written.
         *
         * This value may be inaccurate if the appended var buffers are
         * allowed to mutate - they should not.
         */

        size_t
        size() const
        {
            return size_;
        }
    };
}

#endif // AUGRTPP_BUFFER_HPP
