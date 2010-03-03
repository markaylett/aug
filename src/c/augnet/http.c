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
#define AUGNET_BUILD
#include "augnet/http.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/conv.h"
#include "augutil/lexer.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/string.h"

#include <assert.h>

struct aug_httpparser_ {
    aug_mpool* mpool_;
    aug_httphandler* handler_;
    aug_object* ob_;
    aug_lexer_t lexer_;
    char name_[AUG_MAXLINE];
    enum {
        REQUEST_,
        NAME_,
        VALUE_,
        BODY_
    } state_;
    int csize_;
};

static aug_result
request_(aug_httpparser_t parser)
{
    /* Unless Content-Length is encountered, read body until end of stream is
       reached. */

    return aug_httprequest(parser->handler_, aug_lexertoken(parser->lexer_));
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

            if (!aug_strtoui(aug_lexertoken(parser->lexer_), &csize, 10))
                return -1;

            parser->csize_ = (int)csize;
            return aug_httpcsize(parser->handler_, csize);
        }
    }

    return aug_httpfield(parser->handler_, parser->name_,
                         aug_lexertoken(parser->lexer_));
}

static aug_result
end_(aug_httpparser_t parser, aug_bool commit)
{
    parser->state_ = REQUEST_;
    parser->csize_ = 0;
    return aug_httpend(parser->handler_, commit);
}

static aug_result
phrase_(aug_httpparser_t parser)
{
    /* Switch to body if Content-Length has been set. */

    if (parser->csize_) {
        parser->state_ = BODY_;
        return 0;
    }

    /* End of message (with commit). */

    return end_(parser, AUG_TRUE);
}

static aug_result
label_(aug_httpparser_t parser)
{
    /* No label on request line. */

    if (REQUEST_ == parser->state_) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("missing request line"));
        return -1;
    }

    name_(parser);

    /* The field's value follows its name. */

    parser->state_ = VALUE_;
    return 0;
}

static aug_result
word_(aug_httpparser_t parser)
{
    aug_result result;
    switch (parser->state_) {
    case REQUEST_:
        if (0 <= (result = request_(parser)))
            parser->state_ = NAME_;
        break;
    case NAME_:
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("missing field name"));
        result = -1;
        break;
    case VALUE_:
        if (0 <= (result = value_(parser)))
            parser->state_ = NAME_;
        break;
    default:
        result = -1;
        assert(0);
    }
    return result;
}

static aug_rsize
header_(aug_httpparser_t parser, const char* ptr, unsigned size)
{
    unsigned i = 0;
    while (i < size) {

        switch (aug_appendlexer(parser->lexer_, ptr[i++])) {
        case AUG_LEXLABEL:
            if (label_(parser) < 0)
                return -1;
            break;
        case AUG_LEXWORD:
            if (word_(parser) < 0)
                return -1;
            break;
        case AUG_LEXWORD | AUG_LEXPHRASE:
            if (word_(parser) < 0
                || phrase_(parser) < 0)
                return -1;
            goto done;
        case AUG_LEXPHRASE:
            if (phrase_(parser) < 0)
                return -1;
            goto done;
        }
    }
 done:
    return i;
}

static aug_rsize
body_(aug_httpparser_t parser, const char* buf, unsigned size)
{
    if ((int)size < parser->csize_) {

        /* Not enough data to fulfil the content. */

        parser->csize_ -= size;

        if (aug_httpcdata(parser->handler_, buf, size) < 0)
            return -1;

        /* Entire buffer consumed. */

        return size;
    }

    /* Consume enough of the buffer to fulfil content. */

    if ((size = parser->csize_))
        if (aug_httpcdata(parser->handler_, buf, size) < 0)
            return -1;

    /* End of message (with commit). */

    if (end_(parser, AUG_TRUE) < 0)
        return -1;

    return size;
}

AUGNET_API aug_httpparser_t
aug_createhttpparser(aug_mpool* mpool, aug_httphandler* handler,
                     unsigned size)
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
    parser->lexer_ = lexer;
    parser->name_[0] = '\0';
    parser->state_ = REQUEST_;
    parser->csize_ = 0;

    aug_retain(mpool);
    aug_retain(handler);
    return parser;
}

AUGNET_API void
aug_destroyhttpparser(aug_httpparser_t parser)
{
    aug_mpool* mpool = parser->mpool_;
    aug_destroylexer(parser->lexer_);
    aug_release(parser->handler_);
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

        if ((result = header_(parser, buf, size)) < 0)
            break;

        if (result == size)
            return 0;

        buf += result;
        size -= result;

    body:
        if ((result = body_(parser, buf, size)) < 0)
            break;

        if (result == size)
            return 0;

        buf += result;
        size -= result;
    }

    /* End of message (no commit). */

    end_(parser, AUG_FALSE);
    return result;
}

AUGNET_API aug_result
aug_finishhttp(aug_httpparser_t parser)
{
    aug_result result;
    switch (aug_finishlexer(parser->lexer_)) {
    case AUG_LEXLABEL:
        if ((result = label_(parser)) < 0)
            goto fail;
        break;
    case AUG_LEXWORD:
        if ((result = word_(parser)) < 0)
            goto fail;
        break;
    case AUG_LEXWORD | AUG_LEXPHRASE:
        if ((result = word_(parser)) < 0
            || (result = phrase_(parser)) < 0)
            goto fail;
        break;
    case AUG_LEXPHRASE:
        if ((result = phrase_(parser)) < 0)
            goto fail;
        break;
    }

    if (REQUEST_ != parser->state_) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("partial read of http message"));
        result = 0;
        goto fail;
    }

    return 0;

 fail:

    /* End of message (no commit). */

    end_(parser, AUG_FALSE);
    return result;
}
