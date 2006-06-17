/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/parser.h"

static const char rcsid[] = "$Id:$";

#include "augutil/conv.h"
#include "augutil/lexer.h"

#include "augsys/defs.h" /* AUG_MAXLINE */
#include "augsys/errinfo.h"
#include "augsys/string.h"

#include <errno.h>       /* ENOMEM */
#include <stdlib.h>      /* malloc() */

struct aug_parser_ {
    const struct aug_handlers* handlers_;
    void* arg_;
    aug_lexer_t lexer_;
    enum {
        INITIAL_,
        NAME_,
        VALUE_,
        BODY_
    } state_;
    char name_[AUG_MAXLINE];
    size_t csize_;
};

static int
iscolon_(char ch)
{
    return ':' == ch;
}

static int
setinitial_(aug_parser_t parser)
{
    return (*parser->handlers_->setinitial_)(parser->arg_,
                                             aug_token(parser->lexer_));
}

static int
setname_(aug_parser_t parser)
{
    aug_strlcpy(parser->name_, aug_token(parser->lexer_),
                sizeof(parser->name_));
    return 0;
}

static int
setvalue_(aug_parser_t parser)
{
    size_t csize;

    /* Intercept and handler any fields that affect the parsing phase. */

    switch (parser->name_[0]) {
    case 'C':
    case 'c':
        if (0 == aug_strcasecmp(&parser->name_[1], "ontent-Length")) {

            if (-1 == aug_strtoui(&csize, aug_token(parser->lexer_), 10))
                return -1;

            if (-1 == (*parser->handlers_->setcsize_)(parser->arg_, csize))
                return -1;

            parser->csize_ = csize;
            return 0;
        }
    }

    return (*parser->handlers_->setfield_)(parser->arg_, parser->name_,
                                           aug_token(parser->lexer_));
}

static int
end_(aug_parser_t parser, int commit)
{
    parser->state_ = INITIAL_;
    parser->csize_ = 0;
    return (*parser->handlers_->end_)(parser->arg_, commit);
}

static int
tail_(aug_parser_t parser)
{
    switch (parser->state_) {
    case INITIAL_:

        /* Ignore empty lines that precede a message. */

        if ('\0' != *aug_token(parser->lexer_))
            setinitial_(parser);
        break;

    case NAME_:

        /* Empty line in the name phase of the parse. */

        if ('\0' != *aug_token(parser->lexer_)) {
            aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                           AUG_MSG("failed to parse name"));
            return -1;
        }
        break;

    case VALUE_:
        setvalue_(parser);
        break;

    case BODY_:

        /* Check to ensure that the entire body has been parsed. */

        if (0 < parser->csize_) {
            aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                           AUG_MSG("failed to parse body"));
            return -1;
        }
    }
    return 0;
}

static ssize_t
header_(aug_parser_t parser, const char* ptr, size_t size)
{
    size_t i = 0;
    while (i < size) {

        switch (aug_lexchar(&parser->lexer_, ptr[i++])) {
        case AUG_TOKERROR:
            return -1;
        case AUG_TOKNONE:
            break;
        case AUG_TOKDELIM:
            if (-1 == setname_(parser))
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
    return (ssize_t)i;
}

static ssize_t
body_(aug_parser_t parser, const char* buf, size_t size)
{
    if (size < parser->csize_) {

        /* Not enough data to fulfil the content. */

        if (-1 == (*parser->handlers_->cdata_)(parser->arg_, buf, size))
            return -1;

        parser->csize_ -= size;

        /* Entire buffer consumed. */

        return (ssize_t)size;
    }

    /* Consume enough of the buffer to fulfil content. */

    size = parser->csize_;
    if (-1 == (*parser->handlers_->cdata_)(parser->arg_, buf, size))
        return -1;

    /* End of message (with commit). */

    if (-1 == end_(parser, 1))
        return -1;

    return (ssize_t)size;
}

AUGNET_API aug_parser_t
aug_createparser(size_t size, const struct aug_handlers* handlers, void* arg)
{
    aug_parser_t parser = malloc(sizeof(struct aug_parser_));
    if (!parser) {
        aug_setposixerrinfo(__FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    if (!(parser->lexer_ = aug_createlexer(size, NULL))) {
        free(parser);
        return NULL;
    }

    parser->handlers_ = handlers;
    parser->arg_ = arg;
    parser->state_ = INITIAL_;
    parser->csize_ = 0;
    return parser;
}

AUGNET_API int
aug_freeparser(aug_parser_t parser)
{
    int ret = aug_freelexer(parser->lexer_);
    free(parser);
    return ret;
}

AUGNET_API int
aug_parse(aug_parser_t parser, const char* buf, size_t size)
{
    ssize_t ret;

    if (BODY_ == parser->state_)
        goto body;

    /* Header and body parts of the message are alternately parsed until the
       entire buffer has been consumed. */

    for (;;) {

        if (-1 == (ret = header_(parser, buf, size)))
            break;

        if ((size_t)ret == size)
            return 0;

        buf += ret;
        size -= ret;

    body:
        if (-1 == (ret = body_(parser, buf, size)))
            break;

        if ((size_t)ret == size)
            return 0;

        buf += ret;
        size -= ret;
    }

    /* End of message (no commit). */

    end_(parser, 0);
    return -1;
}

AUGNET_API int
aug_parseend(aug_parser_t parser)
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
