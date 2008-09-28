/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnet.h"
#include "augsys.h"
#include "augctx.h"

static const char TEST_[] =
"GET / HTTP/1.1\r\n"
"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg,"
" application/vnd.ms-excel, application/vnd.ms-powerpoint,"
" application/msword, application/x-shockwave-flash, */*\r\n"
"Accept-Language: en-gb\r\n"
"Accept-Encoding: gzip, deflate\r\n"
"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1;"
" .NET CLR 1.0.3705; .NET CLR 1.1.4322; .NET CLR 2.0.50727)\r\n"
"Host: localhost:8080\r\n"
"Connection: Keep-Alive\r\n\r\n";

static aug_result
setinitial_(aug_object* ob, const char* value)
{
    aug_ctxinfo(aug_tlx, "initial: %s", value);
    return AUG_SUCCESS;
}

static aug_result
setfield_(aug_object* ob, const char* name, const char* value)
{
    aug_ctxinfo(aug_tlx, "field: %s=%s", name, value);
    return AUG_SUCCESS;
}

static aug_result
setcsize_(aug_object* ob, unsigned size)
{
    aug_ctxinfo(aug_tlx, "size: %d", (int)size);
    return AUG_SUCCESS;
}

static aug_result
cdata_(aug_object* ob, const void* buf, unsigned len)
{
    return AUG_SUCCESS;
}

static aug_result
end_(aug_object* ob, int commit)
{
    return AUG_SUCCESS;
}

static const struct aug_httphandler handler_ = {
    setinitial_,
    setfield_,
    setcsize_,
    cdata_,
    end_
};

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    aug_httpparser_t parser;

    if (AUG_ISFAIL(aug_autobasictlx()))
        return 1;

    mpool = aug_getmpool(aug_tlx);
    parser = aug_createhttpparser(mpool, 1024, &handler_, NULL);
    aug_release(mpool);

    if (!parser) {
        aug_perrinfo(aug_tlx, "aug_createhttpparser() failed", NULL);
        return 1;
    }

    if (AUG_ISFAIL(aug_appendhttp(parser, TEST_, sizeof(TEST_) - 1))) {
        aug_perrinfo(aug_tlx, "aug_appendhttp() failed", NULL);
        goto fail;
    }

    if (AUG_ISFAIL(aug_finishhttp(parser))) {
        aug_perrinfo(aug_tlx, "aug_finishhttp() failed", NULL);
        goto fail;
    }

    aug_destroyhttpparser(parser);
    return 0;

 fail:
    aug_destroyhttpparser(parser);
    return 1;
}
