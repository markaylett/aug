/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/mar.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augnet/http.h"

#include "augutil/object.h"
#include "augutil/xstr.h"

#include "augmar/mar.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"

#include <string.h>

struct aug_marparser_ {
    aug_mpool* mpool_;
    aug_marpool* marpool_;
    aug_httphandler httphandler_;
    aug_httpparser_t http_;
    aug_xstr_t initial_;
    aug_mar_t mar_;
};

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
    struct aug_marparser_* parser
        = AUG_PODIMPL(struct aug_marparser_, httphandler_, ob);

    parser->initial_ = aug_createxstr(parser->mpool_, 0);

    if (!parser->initial_)
        return AUG_FAILERROR; /* Allocation failed. */

    if (!(parser->mar_ = aug_getmar(parser->marpool_, initial))) {
        aug_destroyxstr(parser->initial_);
        parser->initial_ = NULL;
        return AUG_FAILERROR;
    }
    aug_xstrcpys(parser->initial_, initial);
    return AUG_SUCCESS;
}

static aug_result
field_(aug_httphandler* ob, const char* name, const char* value)
{
    struct aug_marparser_* parser
        = AUG_PODIMPL(struct aug_marparser_, httphandler_, ob);

    struct aug_field field;

    field.name_ = name;
    field.value_ = value;
    field.size_ = (unsigned)strlen(value);

    return aug_setfield(parser->mar_, &field, NULL);
}

static aug_result
csize_(aug_httphandler* ob, unsigned csize)
{
    struct aug_marparser_* parser
        = AUG_PODIMPL(struct aug_marparser_, httphandler_, ob);

    aug_verify(aug_truncatemar(parser->mar_, csize));
    aug_verify(aug_seekmar(parser->mar_, AUG_SET, 0));

    return AUG_SUCCESS;
}

static aug_result
cdata_(aug_httphandler* ob, const void* buf, unsigned len)
{
    struct aug_marparser_* parser
        = AUG_PODIMPL(struct aug_marparser_, httphandler_, ob);

    /* Returns aug_rsize. */

    aug_verify(aug_writemar(parser->mar_, buf, len));

    return AUG_SUCCESS;
}

static aug_result
end_(aug_httphandler* ob, aug_bool commit)
{
    struct aug_marparser_* parser
        = AUG_PODIMPL(struct aug_marparser_, httphandler_, ob);

    aug_result result;

    if (commit) {

        if (!parser->initial_)
            return AUG_SUCCESS; /* Blank line. */

        result = aug_putmar(parser->marpool_, aug_xstr(parser->initial_),
                            parser->mar_);
    } else
        result = AUG_SUCCESS;

    if (parser->initial_) {
        aug_destroyxstr(parser->initial_);
        parser->initial_ = NULL;
    }

    if (parser->mar_) {
        aug_releasemar(parser->mar_);
        parser->mar_ = NULL;
    }

    return result;
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

AUGNET_API aug_marparser_t
aug_createmarparser(aug_mpool* mpool, aug_marpool* marpool, unsigned size)
{
    aug_marparser_t parser = aug_allocmem(mpool,
                                          sizeof(struct aug_marparser_));
    if (!parser)
        return NULL;

    parser->mpool_ = mpool;
    parser->marpool_ = marpool;
    parser->httphandler_.vtbl_ = &vtbl_;
    parser->httphandler_.impl_ = NULL;
    parser->http_ = NULL;
    parser->initial_ = NULL;
    parser->mar_ = NULL;

    if (!(parser->http_ = aug_createhttpparser(mpool, &parser->httphandler_,
                                               size))) {
        aug_freemem(mpool, parser);
        return NULL;
    }

    aug_retain(mpool);
    aug_retain(marpool);
    return parser;
}

AUGNET_API void
aug_destroymarparser(aug_marparser_t parser)
{
    aug_mpool* mpool = parser->mpool_;

    if (parser->mar_)
        aug_releasemar(parser->mar_);

    if (parser->initial_)
        aug_destroyxstr(parser->initial_);

    aug_destroyhttpparser(parser->http_);
    aug_release(parser->marpool_);

    aug_freemem(mpool, parser);
    aug_release(mpool);
}

AUGNET_API aug_result
aug_appendmar(aug_marparser_t parser, const char* buf, unsigned size)
{
    return aug_appendhttp(parser->http_, buf, size);
}

AUGNET_API aug_result
aug_finishmar(aug_marparser_t parser)
{
    return aug_finishhttp(parser->http_);
}
