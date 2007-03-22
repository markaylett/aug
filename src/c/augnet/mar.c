/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/mar.h"

#include "augnet/http.h"

#include "augutil/strbuf.h"
#include "augutil/var.h"

#include "augmar/mar.h"

#include "augsys/errinfo.h"
#include "augsys/errno.h"

#include <stdlib.h>
#include <string.h>

static const char rcsid[] = "$Id:$";

struct aug_marparser_ {
    const struct aug_marhandler* handler_;
    struct aug_var var_;
    aug_httpparser_t http_;
    aug_strbuf_t initial_;
    aug_mar_t mar_;
};

static int
initial_(const struct aug_var* var, const char* initial)
{
    aug_marparser_t parser = var->arg_;
    if (!(parser->initial_ = aug_createstrbuf(0)))
        return -1;

    if (!(parser->mar_ = parser->handler_->create_(&parser->var_, initial))) {
        aug_destroystrbuf(parser->initial_);
        parser->initial_ = NULL;
        return -1;
    }

    aug_setstrbufs(&parser->initial_, initial);
    return 0;
}

static int
field_(const struct aug_var* var, const char* name, const char* value)
{
    aug_marparser_t parser = var->arg_;
    struct aug_field field;

    field.name_ = name;
    field.value_ = value;
    field.size_ = (unsigned)strlen(value);

    return aug_setfield(parser->mar_, &field, NULL);
}

static int
csize_(const struct aug_var* var, unsigned csize)
{
    aug_marparser_t parser = var->arg_;
    if (-1 == aug_truncatemar(parser->mar_, csize))
        return -1;
    return aug_seekmar(parser->mar_, AUG_SET, 0);
}

static int
cdata_(const struct aug_var* var, const void* buf, unsigned len)
{
    aug_marparser_t parser = var->arg_;
    return aug_writemar(parser->mar_, buf, len);
}

static int
end_(const struct aug_var* var, int commit)
{
    aug_marparser_t parser = var->arg_;
    int ret = 0;
    if (commit) {

        if (!parser->initial_)
            return 0; /* Blank line. */

        ret = parser->handler_
            ->message_(&parser->var_, aug_getstr(parser->initial_),
                       parser->mar_);
    }

    if (parser->initial_) {
        aug_destroystrbuf(parser->initial_);
        parser->initial_ = NULL;
    }

    if (parser->mar_) {
        aug_releasemar(parser->mar_);
        parser->mar_ = NULL;
    }

    return ret;
}

static const struct aug_httphandler handler_ = {
    initial_,
    field_,
    csize_,
    cdata_,
    end_
};

AUGNET_API aug_marparser_t
aug_createmarparser(unsigned size, const struct aug_marhandler* handler,
                    const struct aug_var* var)
{
    aug_marparser_t parser = malloc(sizeof(struct aug_marparser_));
    struct aug_var local;

    if (!parser) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    parser->handler_ = handler;
    aug_setvar(&parser->var_, var);
    local.type_ = NULL;
    local.arg_ = parser;
    if (!(parser->http_ = aug_createhttpparser(size, &handler_, &local)))
        goto fail;

    parser->initial_ = NULL;
    parser->mar_ = NULL;
    return parser;

 fail:
    free(parser);
    return NULL;
}

AUGNET_API int
aug_destroymarparser(aug_marparser_t parser)
{
    aug_destroyvar(&parser->var_);
    aug_destroyhttpparser(parser->http_);

    if (parser->initial_)
        aug_destroystrbuf(parser->initial_);

    if (parser->mar_)
        aug_releasemar(parser->mar_);

    free(parser);
    return 0;
}

AUGNET_API int
aug_appendmar(aug_marparser_t parser, const char* buf, unsigned size)
{
    return aug_appendhttp(parser->http_, buf, size);
}

AUGNET_API int
aug_finishmar(aug_marparser_t parser)
{
    return aug_finishhttp(parser->http_);
}
