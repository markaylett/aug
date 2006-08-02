/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
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

static void
setinitial_(const struct aug_var* arg, const char* value)
{
    aug_info("initial: %s", value);
}

static void
setfield_(const struct aug_var* arg, const char* name, const char* value)
{
    aug_info("field: %s=%s", name, value);
}

static void
setcsize_(const struct aug_var* arg, size_t size)
{
    aug_info("size: %d", (int)size);
}

static void
cdata_(const struct aug_var* arg, const void* buf, size_t size)
{
}

static void
end_(const struct aug_var* arg, int commit)
{
}

static const struct aug_httphandlers handlers_ = {
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

    if (!(parser = aug_createhttpparser(1024, &handlers_, NULL))) {
        aug_perrinfo("aug_createhttpparser() failed");
        return 1;
    }

    if (-1 == aug_parsehttp(parser, TEST_, sizeof(TEST_) - 1)) {
        aug_perrinfo("aug_parsehttp() failed");
        goto fail;
    }

    if (-1 == aug_endhttp(parser)) {
        aug_perrinfo("aug_endhttp() failed");
        goto fail;
    }

    if (-1 == aug_freehttpparser(parser)) {
        aug_perrinfo("aug_freehttpparser() failed");
        return 1;
    }
    return 0;

 fail:
    aug_freehttpparser(parser);
    return 1;
}
