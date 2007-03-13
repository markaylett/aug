/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/http.h"

static const char rcsid[] = "$Id$";

#include "augutil/conv.h"
#include "augutil/lexer.h"
#include "augutil/var.h"

#include "augsys/defs.h" /* AUG_MAXLINE */
#include "augsys/errinfo.h"
#include "augsys/string.h"

#include <errno.h>       /* ENOMEM */
#include <stdlib.h>      /* malloc() */

#define EOF_ (-1)

struct aug_httpparser_ {
    const struct aug_httphandler* handler_;
    struct aug_var var_;
    aug_lexer_t lexer_;
    enum {
        INITIAL_,
        NAME_,
        VALUE_,
        BODY_
    } state_;
    char name_[AUG_MAXLINE];
    int csize_;
};

static int
iscolon_(char ch)
{
    return ':' == ch;
}

static int
initial_(aug_httpparser_t parser)
{
    /* Unless Content-Length is encountered, read body until end of stream is
       reached. */

    /* parser->csize_ = EOF_; */
    return parser->handler_
        ->initial_(&parser->var_, aug_token(parser->lexer_));
}

static int
name_(aug_httpparser_t parser)
{
    aug_strlcpy(parser->name_, aug_token(parser->lexer_),
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
        if (0 == aug_strcasecmp(&parser->name_[1], "ontent-Length")) {

            if (-1 == aug_strtoui(&csize, aug_token(parser->lexer_), 10))
                return -1;

            parser->csize_ = (int)csize;
            return parser->handler_->csize_(&parser->var_, csize);
        }
    }

    return parser->handler_
        ->field_(&parser->var_, parser->name_, aug_token(parser->lexer_));
}

static int
end_(aug_httpparser_t parser, int commit)
{
    parser->state_ = INITIAL_;
    parser->csize_ = 0;
    return parser->handler_->end_(&parser->var_, commit);
}

static int
tail_(aug_httpparser_t parser)
{
    int ret = 0;
    switch (parser->state_) {
    case INITIAL_:

        /* Ignore empty lines that precede a message. */

        if ('\0' != *aug_token(parser->lexer_))
            ret = initial_(parser);
        break;

    case NAME_:

        /* Empty line in the name phase of the parse. */

        if ('\0' != *aug_token(parser->lexer_)) {
            aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                           AUG_MSG("failed to parse name"));
            ret = -1;
        }
        break;

    case VALUE_:
        ret = value_(parser);
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

static int
header_(aug_httpparser_t parser, const char* ptr, unsigned size)
{
    unsigned i = 0;
    while (i < size) {

        switch (aug_lexchar(&parser->lexer_, ptr[i++])) {
        case AUG_TOKERROR:
            return -1;
        case AUG_TOKNONE:
            break;
        case AUG_TOKDELIM:
            if (-1 == name_(parser))
                return -1;

            /* The field's value follows its name. */

            parser->state_ = VALUE_;
            aug_setisdelim(parser->lexer_, NULL);
            break;

        case AUG_TOKLINE:
            if (-1 == tail_(parser))
                return -1;

            parser->state_ = NAME_;
            aug_setisdelim(parser->lexer_, iscolon_);
            break;

        case AUG_TOKBREAK:
            if (-1 == tail_(parser))
                return -1;

            if (0 == parser->csize_) {

                /* End of message (with commit). */

                if (-1 == end_(parser, 1))
                    return -1;
            } else
                parser->state_ = BODY_;
            goto done;
        }
    }

 done:
    return (int)i;
}

static int
body_(aug_httpparser_t parser, const char* buf, unsigned size)
{
    if (EOF_ == parser->csize_ || (int)size < parser->csize_) {

        /* Not enough data to fulfil the content. */

        if (EOF_ != parser->csize_)
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

    if (!(lexer = aug_createlexer(size, NULL))) {
        free(parser);
        return NULL;
    }

    parser->handler_ = handler;
    aug_setvar(&parser->var_, var);
    parser->lexer_ = lexer;
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
aug_parsehttp(aug_httpparser_t parser, const char* buf, unsigned size)
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
aug_endhttp(aug_httpparser_t parser)
{
    if (-1 == aug_lexend(&parser->lexer_))
        goto fail;

    if (-1 == tail_(parser))
        goto fail;

    return 0;

 fail:

    /* End of message (no commit). */

    end_(parser, 0);
    return -1;
}
