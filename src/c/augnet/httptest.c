/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnet.h"
#include "augsys.h"

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

static int
setinitial_(aub_object* ob, const char* value)
{
    aug_info("initial: %s", value);
    return 0;
}

static int
setfield_(aub_object* ob, const char* name, const char* value)
{
    aug_info("field: %s=%s", name, value);
    return 0;
}

static int
setcsize_(aub_object* ob, unsigned size)
{
    aug_info("size: %d", (int)size);
    return 0;
}

static int
cdata_(aub_object* ob, const void* buf, unsigned len)
{
    return 0;
}

static int
end_(aub_object* ob, int commit)
{
    return 0;
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
    struct aug_errinfo errinfo;
    aug_httpparser_t parser;

    aug_atexitinit(&errinfo);

    if (!(parser = aug_createhttpparser(1024, &handler_, NULL))) {
        aug_perrinfo(NULL, "aug_createhttpparser() failed");
        return 1;
    }

    if (-1 == aug_appendhttp(parser, TEST_, sizeof(TEST_) - 1)) {
        aug_perrinfo(NULL, "aug_appendhttp() failed");
        goto fail;
    }

    if (-1 == aug_finishhttp(parser)) {
        aug_perrinfo(NULL, "aug_finishhttp() failed");
        goto fail;
    }

    if (-1 == aug_destroyhttpparser(parser)) {
        aug_perrinfo(NULL, "aug_destroyhttpparser() failed");
        return 1;
    }
    return 0;

 fail:
    aug_destroyhttpparser(parser);
    return 1;
}
