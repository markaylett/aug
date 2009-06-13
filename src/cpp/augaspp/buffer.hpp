/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGASPP_BUFFER_HPP
#define AUGASPP_BUFFER_HPP

#include "augaspp/config.hpp"
#include "augsyspp/types.hpp"
#include "augnetpp/writer.hpp"

#include <vector>

#define AUG_VARBUFFER 1

namespace aug {

    namespace detail {
        class vecblob : public mpool_ops {
            blob<vecblob> blob_;
            std::vector<char> vec_;
            size_t size_;
            int refs_;
        public:
            explicit
            vecblob(size_t size)
                : vec_(size),
                  size_(0),
                  refs_(1)
            {
                blob_.reset(this);
            }
            objectptr
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
            const char*
            getblobtype_()
            {
                return "application/octet-stream";
            }
            const void*
            getblobdata_(size_t& size) AUG_NOTHROW
            {
                size = size_;
                return &vec_[0];
            }
            size_t
            getblobsize_() AUG_NOTHROW
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

    class buffer : public mpool_ops {
        writer writer_;
        detail::vecblob blob_;
        bool reuse_;
        size_t size_;

    public:
        ~buffer() AUG_NOTHROW;

        explicit
        buffer(mpoolref mpool, size_t size = 1024);

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
        writesome(streamref ref);

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

#endif // AUGASPP_BUFFER_HPP
