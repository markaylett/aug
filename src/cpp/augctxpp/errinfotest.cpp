/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctxpp.hpp"

#include "augext.h"

using namespace aug;

namespace {

    class test : public ref_base {
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
        smartob<aug_object>
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
        readv_(const struct iovec* iov, int size) AUG_NOTHROW
        {
            return AUG_MKRESULT(size);
        }
        aug_rsize
        write_(const void* buf, size_t size) AUG_NOTHROW
        {
            return AUG_MKRESULT(size);
        }
        aug_rsize
        writev_(const struct iovec* iov, int size) AUG_NOTHROW
        {
            return AUG_MKRESULT(size);
        }
        void
        copyerrinfo_(struct aug_errinfo& dst) AUG_NOTHROW
        {
            aug_seterrinfo(&dst, __FILE__, __LINE__, "aug", AUG_EASSERT,
                           "some error");
        }
        static smartob<aug_stream>
        create()
        {
            test* ptr = new test();
            return object_attach<aug_stream>(ptr->stream_);
        }
    };
}

int
main(int argc, char* argv[])
{
    try {

        autobasictlx();
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
