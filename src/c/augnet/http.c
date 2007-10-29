/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/http.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augutil/conv.h"
#include "augutil/lexer.h"
#include "augutil/types.h"
#include "augutil/var.h"

#include "augsys/errinfo.h"
#include "augsys/string.h"

#include <assert.h>
#include <errno.h>  /* ENOMEM */
#include <stdlib.h> /* malloc() */

struct aug_httpparser_ {
    const struct aug_httphandler* handler_;
    struct aug_var var_;
    aug_lexer_t lexer_;
    char name_[AUG_MAXLINE];
    enum {
        INITIAL_,
        NAME_,
        VALUE_,
        BODY_
    } state_;
    int csize_;
};

static int
initial_(aug_httpparser_t parser)
{
    /* Unless Content-Length is encountered, read body until end of stream is
       reached. */

    return parser->handler_
        ->initial_(&parser->var_, aug_lexertoken(parser->lexer_));
}

static int
name_(aug_httpparser_t parser)
{
    aug_strlcpy(parser->name_, aug_lexertoken(parser->lexer_),
                sizeof(parser->name_));
    return 0;
}

static int
value_(aug_httpparser_t parser)
{
    unsigned csize;

    /* Intercept and handle any fields that affect the parsing phase. */

    switch (parser->name_[0]) {
    case 'C':
    case 'c':
        if (0 == aug_strcasecmp(parser->name_ + 1, "ontent-Length")) {

            if (-1 == aug_strtoui(&csize, aug_lexertoken(parser->lexer_), 10))
                return -1;

            parser->csize_ = (int)csize;
            return parser->handler_->csize_(&parser->var_, csize);
        }
    }

    return parser->handler_->field_(&parser->var_, parser->name_,
                                    aug_lexertoken(parser->lexer_));
}

static int
end_(aug_httpparser_t parser, int commit)
{
    parser->state_ = INITIAL_;
    parser->csize_ = 0;
    return parser->handler_->end_(&parser->var_, commit);
}

static int
header_(aug_httpparser_t parser, const char* ptr, unsigned size)
{
    unsigned i = 0;
    while (i < size) {

        switch (aug_appendlexer(parser->lexer_, ptr[i++])) {
        case AUG_TOKPHRASE:

            /* Switch to body if Content-Length has been set. */

            if (parser->csize_) {
                parser->state_ = BODY_;
            } else {

                /* End of message (with commit). */

                if (-1 == end_(parser, 1))
                    return -1;
            }
            goto done;
        case AUG_TOKLABEL:

            /* No label on initial line. */

            if (INITIAL_ == parser->state_) {
                aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL,
                               AUG_EPARSE, AUG_MSG("missing initial line"));
                return -1;
            }

            if (-1 == name_(parser))
                return -1;

            /* The field's value follows its name. */

            parser->state_ = VALUE_;
            break;
        case AUG_TOKWORD:
            switch (parser->state_) {
            case INITIAL_:
                if (-1 == initial_(parser))
                    return -1;
                parser->state_ = NAME_;
                break;
            case NAME_:
                aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL,
                               AUG_EPARSE, AUG_MSG("missing field name"));
                break;
            case VALUE_:
                if (-1 == value_(parser))
                    return -1;
                parser->state_ = NAME_;
                break;
            default:
                assert(0);
            }
            break;
        }
    }
done:
    return (int)i;
}

static int
body_(aug_httpparser_t parser, const char* buf, unsigned size)
{
    if ((int)size < parser->csize_) {

        /* Not enough data to fulfil the content. */

        parser->csize_ -= size;

        if (-1 == parser->handler_->cdata_(&parser->var_, buf, size))
            return -1;

        /* Entire buffer consumed. */

        return size;
    }

    /* Consume enough of the buffer to fulfil content. */

    if ((size = parser->csize_)
        && -1 == parser->handler_->cdata_(&parser->var_, buf, size))
        return -1;

    /* End of message (with commit). */

    if (-1 == end_(parser, 1))
        return -1;

    return size;
}

static int
finish_(aug_httpparser_t parser)
{
    int ret = 0;
    switch (parser->state_) {
    case INITIAL_:
    case NAME_:
    case VALUE_:
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("failed to parse header"));
        ret = -1;
        break;
    case BODY_:

        /* Check to ensure that the entire body has been parsed. */

        if (0 < parser->csize_) {

            aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                           AUG_MSG("failed to parse body"));
            ret = -1;

        } else if (-1 == parser->csize_) {

            ret = end_(parser, 1);
        }
        break;
    }
    return ret;
}

AUGNET_API aug_httpparser_t
aug_createhttpparser(unsigned size, const struct aug_httphandler* handler,
                     const struct aug_var* var)
{
    aug_httpparser_t parser = malloc(sizeof(struct aug_httpparser_));
    aug_lexer_t lexer;

    if (!parser) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    if (!(lexer = aug_createnetlexer(size))) {
        free(parser);
        return NULL;
    }

    parser->handler_ = handler;
    aug_setvar(&parser->var_, var);
    parser->lexer_ = lexer;
    parser->name_[0] = '\0';
    parser->state_ = INITIAL_;
    parser->csize_ = 0;
    return parser;
}

AUGNET_API int
aug_destroyhttpparser(aug_httpparser_t parser)
{
    int ret = aug_destroylexer(parser->lexer_);
    aug_destroyvar(&parser->var_);
    free(parser);
    return ret;
}

AUGNET_API int
aug_appendhttp(aug_httpparser_t parser, const char* buf, unsigned size)
{
    int ret;

    if (BODY_ == parser->state_)
        goto body;

    /* Header and body parts of the message are alternately parsed until the
       entire buffer has been consumed. */

    for (;;) {

        if (-1 == (ret = header_(parser, buf, size)))
            break;

        if (ret == size)
            return 0;

        buf += ret;
        size -= ret;

    body:
        if (-1 == (ret = body_(parser, buf, size)))
            break;

        if (ret == size)
            return 0;

        buf += ret;
        size -= ret;
    }

    /* End of message (no commit). */

    end_(parser, 0);
    return -1;
}

AUGNET_API int
aug_finishhttp(aug_httpparser_t parser)
{
    if (-1 == aug_finishlexer(parser->lexer_))
        goto fail;

    if (-1 == finish_(parser))
        goto fail;

    return 0;

 fail:

    /* End of message (no commit). */

    end_(parser, 0);
    return -1;
}
