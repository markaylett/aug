/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/http.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/conv.h"
#include "augutil/lexer.h"
#include "augutil/types.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/string.h"

#include <assert.h>

struct aug_httpparser_ {
    aug_mpool* mpool_;
    const struct aug_httphandler* handler_;
    aug_object* ob_;
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

static aug_result
initial_(aug_httpparser_t parser)
{
    /* Unless Content-Length is encountered, read body until end of stream is
       reached. */

    return parser->handler_
        ->initial_(parser->ob_, aug_lexertoken(parser->lexer_));
}

static void
name_(aug_httpparser_t parser)
{
    aug_strlcpy(parser->name_, aug_lexertoken(parser->lexer_),
                sizeof(parser->name_));
}

static aug_result
value_(aug_httpparser_t parser)
{
    unsigned csize;

    /* Intercept and handle any fields that affect the parsing phase. */

    switch (parser->name_[0]) {
    case 'C':
    case 'c':
        if (0 == aug_strcasecmp(parser->name_ + 1, "ontent-Length")) {

            if (!aug_strtoui(&csize, aug_lexertoken(parser->lexer_), 10))
                return AUG_FAILERROR;

            parser->csize_ = (int)csize;
            return parser->handler_->csize_(parser->ob_, csize);
        }
    }

    return parser->handler_->field_(parser->ob_, parser->name_,
                                    aug_lexertoken(parser->lexer_));
}

static aug_result
end_(aug_httpparser_t parser, int commit)
{
    parser->state_ = INITIAL_;
    parser->csize_ = 0;
    return parser->handler_->end_(parser->ob_, commit);
}

static aug_result
phrase_(aug_httpparser_t parser)
{
    /* Switch to body if Content-Length has been set. */

    if (parser->csize_) {
        parser->state_ = BODY_;
        return AUG_SUCCESS;
    }

    /* End of message (with commit). */

    return end_(parser, 1);
}

static aug_result
label_(aug_httpparser_t parser)
{
    /* No label on initial line. */

    if (INITIAL_ == parser->state_) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EPARSE,
                       AUG_MSG("missing initial line"));
        return AUG_FAILERROR;
    }

    name_(parser);

    /* The field's value follows its name. */

    parser->state_ = VALUE_;
    return AUG_SUCCESS;
}

static aug_result
word_(aug_httpparser_t parser)
{
    aug_result result;
    switch (parser->state_) {
    case INITIAL_:
        if (AUG_ISSUCCESS(result = initial_(parser)))
            parser->state_ = NAME_;
        break;
    case NAME_:
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EPARSE,
                       AUG_MSG("missing field name"));
        result = AUG_FAILERROR;
        break;
    case VALUE_:
        if (AUG_ISSUCCESS(result = value_(parser)))
            parser->state_ = NAME_;
        break;
    default:
        assert(0);
    }
    return result;
}

static aug_rsize
header_(aug_httpparser_t parser, const char* ptr, unsigned size)
{
    unsigned i = 0;
    while (i < size) {

        aug_result result;
        switch (aug_appendlexer(parser->lexer_, ptr[i++])) {
        case AUG_LEXLABEL:
            if (AUG_ISFAIL(result = label_(parser)))
                return result;
            break;
        case AUG_LEXWORD:
            if (AUG_ISFAIL(result = word_(parser)))
                return result;
            break;
        case AUG_LEXWORD | AUG_LEXPHRASE:
            if (AUG_ISFAIL(result = word_(parser))
                || AUG_ISFAIL(result = phrase_(parser)))
                return result;
            goto done;
        case AUG_LEXPHRASE:
            if (AUG_ISFAIL(result = phrase_(parser)))
                return result;
            goto done;
        }
    }
 done:
    return AUG_MKRESULT(i);
}

static aug_rsize
body_(aug_httpparser_t parser, const char* buf, unsigned size)
{
    aug_result result;

    if ((int)size < parser->csize_) {

        /* Not enough data to fulfil the content. */

        parser->csize_ -= size;

        if (AUG_ISFAIL(result = parser->handler_->cdata_(parser->ob_, buf,
                                                         size)))
            return result;

        /* Entire buffer consumed. */

        return AUG_MKRESULT(size);
    }

    /* Consume enough of the buffer to fulfil content. */

    if ((size = parser->csize_)
        && AUG_ISFAIL(result = parser->handler_->cdata_(parser->ob_, buf,
                                                        size)))
        return result;

    /* End of message (with commit). */

    if (AUG_ISFAIL(result = end_(parser, 1)))
        return result;

    return AUG_MKRESULT(size);
}

AUGNET_API aug_httpparser_t
aug_createhttpparser(aug_mpool* mpool, unsigned size,
                     const struct aug_httphandler* handler, aug_object* ob)
{
    aug_httpparser_t parser;
    aug_lexer_t lexer;

    if (!(parser = aug_allocmem(mpool, sizeof(struct aug_httpparser_))))
        return NULL;

    if (!(lexer = aug_createnetlexer(mpool, size))) {
        aug_freemem(mpool, parser);
        return NULL;
    }

    parser->mpool_ = mpool;
    parser->handler_ = handler;
    if ((parser->ob_ = ob))
        aug_retain(ob);
    parser->lexer_ = lexer;
    parser->name_[0] = '\0';
    parser->state_ = INITIAL_;
    parser->csize_ = 0;

    aug_retain(mpool);
    return parser;
}

AUGNET_API void
aug_destroyhttpparser(aug_httpparser_t parser)
{
    aug_mpool* mpool = parser->mpool_;
    aug_destroylexer(parser->lexer_);
    if (parser->ob_)
        aug_release(parser->ob_);
    aug_freemem(mpool, parser);
    aug_release(mpool);
}

AUGNET_API aug_result
aug_appendhttp(aug_httpparser_t parser, const char* buf, unsigned size)
{
    aug_result result;

    if (BODY_ == parser->state_)
        goto body;

    /* Header and body parts of the message are alternately parsed until the
       entire buffer has been consumed. */

    for (;;) {

        if (AUG_ISFAIL(result = header_(parser, buf, size)))
            break;

        if (AUG_RESULT(result) == size)
            return AUG_SUCCESS;

        buf += AUG_RESULT(result);
        size -= AUG_RESULT(result);

    body:
        if (AUG_ISFAIL(result = body_(parser, buf, size)))
            break;

        if (AUG_RESULT(result) == size)
            return AUG_SUCCESS;

        buf += AUG_RESULT(result);
        size -= AUG_RESULT(result);
    }

    /* End of message (no commit). */

    end_(parser, 0);
    return result;
}

AUGNET_API aug_result
aug_finishhttp(aug_httpparser_t parser)
{
    switch (aug_finishlexer(parser->lexer_)) {
    case AUG_LEXLABEL:
        if (-1 == label_(parser))
            goto fail;
        break;
    case AUG_LEXWORD:
        if (-1 == word_(parser))
            goto fail;
        break;
    case AUG_LEXWORD | AUG_LEXPHRASE:
        if (-1 == word_(parser)
            || -1 == phrase_(parser))
            goto fail;
        break;
    case AUG_LEXPHRASE:
        if (-1 == phrase_(parser))
            goto fail;
        break;
    }

    if (INITIAL_ != parser->state_) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EPARSE,
                       AUG_MSG("partial read of http message"));
        goto fail;
    }

    return 0;

 fail:

    /* End of message (no commit). */

    end_(parser, 0);
    return -1;
}
