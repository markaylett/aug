/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/mar.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augnet/http.h"

#include "augutil/object.h"
#include "augutil/xstr.h"

#include "augmar/mar.h"

#include "augsys/errinfo.h"
#include "augsys/errno.h"

#include <stdlib.h>
#include <string.h>

struct aug_marparser_ {
    const struct aug_marhandler* handler_;
    aug_object* ob_;
    aug_httpparser_t http_;
    aug_xstr_t initial_;
    aug_mar_t mar_;
};

static int
initial_(aug_object* ob, const char* initial)
{
    aug_marparser_t parser = aug_obtoaddr(ob);
    if (!(parser->initial_ = aug_createxstr(0)))
        return -1;

    if (!(parser->mar_ = parser->handler_->create_(parser->ob_, initial))) {
        aug_destroyxstr(parser->initial_);
        parser->initial_ = NULL;
        return -1;
    }

    aug_xstrcpys(&parser->initial_, initial);
    return 0;
}

static int
field_(aug_object* ob, const char* name, const char* value)
{
    aug_marparser_t parser = aug_obtoaddr(ob);
    struct aug_field field;

    field.name_ = name;
    field.value_ = value;
    field.size_ = (unsigned)strlen(value);

    return aug_setfield(parser->mar_, &field, NULL);
}

static int
csize_(aug_object* ob, unsigned csize)
{
    aug_marparser_t parser = aug_obtoaddr(ob);
    if (-1 == aug_truncatemar(parser->mar_, csize))
        return -1;
    return aug_seekmar(parser->mar_, AUG_SET, 0);
}

static int
cdata_(aug_object* ob, const void* buf, unsigned len)
{
    aug_marparser_t parser = aug_obtoaddr(ob);
    return aug_writemar(parser->mar_, buf, len);
}

static int
end_(aug_object* ob, int commit)
{
    aug_marparser_t parser = aug_obtoaddr(ob);
    int ret = 0;
    if (commit) {

        if (!parser->initial_)
            return 0; /* Blank line. */

        ret = parser->handler_
            ->message_(parser->ob_, aug_xstr(parser->initial_),
                       parser->mar_);
    }

    if (parser->initial_) {
        aug_destroyxstr(parser->initial_);
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

static void
destroy_(void* ptr)
{
    aug_marparser_t parser = ptr;

    if (parser->ob_)
        aug_release(parser->ob_);

    if (parser->http_)
        aug_destroyhttpparser(parser->http_);

    if (parser->initial_)
        aug_destroyxstr(parser->initial_);

    if (parser->mar_)
        aug_releasemar(parser->mar_);

    free(parser);
}

AUGNET_API aug_marparser_t
aug_createmarparser(unsigned size, const struct aug_marhandler* handler,
                    aug_object* ob)
{
    aug_marparser_t parser = malloc(sizeof(struct aug_marparser_));
    aug_object* addrob;

    if (!parser) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    if (!(addrob = (aug_object*)aug_createaddrob(parser, destroy_))) {
        free(parser);
        return NULL;
    }

    /* The addrob now owns this reference: it will be released by
       destroy_(). */

    if ((parser->ob_ = ob))
        aug_retain(ob);

    parser->http_ = aug_createhttpparser(size, &handler_, addrob);
    parser->initial_ = NULL;
    parser->mar_ = NULL;

    /* If created, http parser will hold reference.  Otherwise, it will be
       destroyed now. */

    aug_release(addrob);

    return parser->http_ ? parser : NULL;
}

AUGNET_API int
aug_destroymarparser(aug_marparser_t parser)
{
    aug_httpparser_t http = parser->http_;
    /* Avoid freeing twice, when destroy_() is called. */
    parser->http_ = NULL;
    aug_destroyhttpparser(http);

    /* destroy_() will be called when "ob_" is released. */

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
