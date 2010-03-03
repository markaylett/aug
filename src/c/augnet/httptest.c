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
#include "augnet.h"
#include "augsys.h"
#include "augctx.h"

static const char TEST1_[] =
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

static const char TEST2_[] =
"GET / HTTP/1.1\r\n"
"Host: localhost:8080\r\n"
"User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-GB; rv:1.9.0.6)"
" Gecko/2009011913 Firefox/3.0.6\r\n"
"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
"Accept-Language: en-gb,en;q=0.5\r\n"
"Accept-Encoding: gzip,deflate\r\n"
"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
"Keep-Alive: 300\r\n"
"Connection: keep-alive\r\n\r\n";

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
request_(aug_httphandler* ob, const char* request)
{
    aug_ctxinfo(aug_tlx, "request: %s", request);
    return 0;
}

static aug_result
field_(aug_httphandler* ob, const char* name, const char* value)
{
    aug_ctxinfo(aug_tlx, "field: %s=%s", name, value);
    return 0;
}

static aug_result
csize_(aug_httphandler* ob, unsigned csize)
{
    aug_ctxinfo(aug_tlx, "size: %d", (int)csize);
    return 0;
}

static aug_result
cdata_(aug_httphandler* ob, const void* buf, unsigned len)
{
    return 0;
}

static aug_result
end_(aug_httphandler* ob, aug_bool commit)
{
    return 0;
}

static const struct aug_httphandlervtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    request_,
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

    if (!aug_autotlx())
        return 1;

    mpool = aug_getmpool(aug_tlx);
    parser = aug_createhttpparser(mpool, &httphandler_, 1024);
    aug_release(mpool);

    if (!parser) {
        aug_perrinfo(aug_tlx, "aug_createhttpparser() failed", NULL);
        return 1;
    }

    if (aug_appendhttp(parser, TEST1_, sizeof(TEST1_) - 1) < 0) {
        aug_perrinfo(aug_tlx, "aug_appendhttp() failed", NULL);
        goto fail;
    }

    if (aug_finishhttp(parser) < 0) {
        aug_perrinfo(aug_tlx, "aug_finishhttp() failed", NULL);
        goto fail;
    }

    if (aug_appendhttp(parser, TEST2_, sizeof(TEST2_) - 1) < 0) {
        aug_perrinfo(aug_tlx, "aug_appendhttp() failed", NULL);
        goto fail;
    }

    if (aug_finishhttp(parser) < 0) {
        aug_perrinfo(aug_tlx, "aug_finishhttp() failed", NULL);
        goto fail;
    }

    aug_destroyhttpparser(parser);
    return 0;

 fail:
    aug_destroyhttpparser(parser);
    return 1;
}
