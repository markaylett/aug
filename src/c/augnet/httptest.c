/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
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

static void*
cast_(aug_httphandler* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_httphandlerid)) {
        aug_retain(ob);
        return ob;
    }
    return NULL;
}

static void
retain_(aug_httphandler* ob)
{
}

static void
release_(aug_httphandler* ob)
{
}

static aug_result
initial_(aug_httphandler* ob, const char* initial)
{
    aug_ctxinfo(aug_tlx, "initial: %s", initial);
    return AUG_SUCCESS;
}

static aug_result
field_(aug_httphandler* ob, const char* name, const char* value)
{
    aug_ctxinfo(aug_tlx, "field: %s=%s", name, value);
    return AUG_SUCCESS;
}

static aug_result
csize_(aug_httphandler* ob, unsigned csize)
{
    aug_ctxinfo(aug_tlx, "size: %d", (int)csize);
    return AUG_SUCCESS;
}

static aug_result
cdata_(aug_httphandler* ob, const void* buf, unsigned len)
{
    return AUG_SUCCESS;
}

static aug_result
end_(aug_httphandler* ob, aug_bool commit)
{
    return AUG_SUCCESS;
}

static const struct aug_httphandlervtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    initial_,
    field_,
    csize_,
    cdata_,
    end_
};

static aug_httphandler httphandler_ = { &vtbl_, NULL };

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    aug_httpparser_t parser;

    if (!aug_autodltlx())
        return 1;

    mpool = aug_getmpool(aug_tlx);
    parser = aug_createhttpparser(mpool, &httphandler_, 1024);
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
