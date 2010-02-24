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
#include "augctxpp.hpp"

#include "augext.h"

#include <cstdio>

using namespace aug;
using namespace std;

namespace {

    // Multiple interfaces.

    class test : public ref_base, public mpool_ops {
        stream<test> stream_;
        err<test> err_;
        ~test() AUG_NOTHROW
        {
        }
        test()
        {
            stream_.reset(this);
            err_.reset(this);
        }
    public:
        objectptr
        cast_(const char* id) AUG_NOTHROW
        {
            if (equalid<aug_object>(id) || equalid<aug_stream>(id))
                return object_retain<aug_object>(stream_);
            else if (equalid<aug_err>(id))
                return object_retain<aug_object>(err_);
            return null;
        }
        aug_result
        shutdown_() AUG_NOTHROW
        {
            return AUG_FAILERROR;
        }
        aug_rsize
        read_(void* buf, size_t size) AUG_NOTHROW
        {
            return AUG_MKRESULT(size);
        }
        aug_rsize
        readv_(const iovec* iov, int size) AUG_NOTHROW
        {
            return AUG_MKRESULT(size);
        }
        aug_rsize
        write_(const void* buf, size_t size) AUG_NOTHROW
        {
            return AUG_MKRESULT(size);
        }
        aug_rsize
        writev_(const iovec* iov, int size) AUG_NOTHROW
        {
            return AUG_MKRESULT(size);
        }
        static streamptr
        create()
        {
            test* ptr = new (tlx) test();
            return object_attach<aug_stream>(ptr->stream_);
        }
    };
}

int
main(int argc, char* argv[])
{
    try {

        autotlx();
        streamptr ptr(test::create());
        try {
            shutdown(ptr);
            aug_check(!"exception not thrown");
        } catch (const errinfo_error& e) {
            aug_check(0 == strcmp(e.what(), "some error"));
        }
        return 0;

    } AUG_PERRINFOCATCH;
    return 1;
}
