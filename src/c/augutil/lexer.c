/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/lexer.h"

#include "augsys/defs.h" /* AUG_BUFSIZE */

#include <ctype.h>       /* isspace() */
#include <errno.h>
#include <stdlib.h>      /* malloc() */

#define LTRIM_ (-1)
#define DATA_ (-2)
#define ESC_ (-3)
#define CR_ (-4)
#define LF_ (-5)

struct aug_lexer_ {
    size_t pos_, size_;
    aug_isdelim_t isdelim_;
    int state_;
};

/* The buffer is located immediately after the struct. */

#define BUFFER_(p) ((char*)p + sizeof(struct aug_lexer_))

static int
grow_(aug_lexer_t* lexer)
{
    /* Double the size of the existing buffer. */

    size_t size = (*lexer)->size_ * 2;
    aug_lexer_t ptr = realloc(*lexer, sizeof(struct aug_lexer_) + size);
    if (!ptr)
        return -1;

    ptr->size_ = size;
    *lexer = ptr;
    return 0;
}

static int
islws_(char ch)
{
    switch (ch) {
    case '\t':
        /* No cases for '\r' and '\n'. */
    case '\v':
    case '\f':
    case ' ':
        return 1;
    }
    return 0;
}

static int
append_(aug_lexer_t* lexer, char ch)
{
    /* Grow if necessary. */

    if ((*lexer)->pos_ == (*lexer)->size_ && -1 == grow_(lexer))
        return -1;

    BUFFER_(*lexer)[(*lexer)->pos_++] = ch;
    return 0;
}

static char
pending_(aug_lexer_t lexer)
{
    /* The first character of new line is stored as the state; this must be a
       non-whitespace character - part of the body - otherwise it would have
       been a line continuation. */

    char ch = lexer->state_;
    lexer->state_ = DATA_;
    return ch;
}

static void
rtrim_(aug_lexer_t lexer)
{
    /* Find last non-space character, or beginning of buffer if none exist. */

    size_t i = lexer->pos_;
    while (0 < i && isspace((int)(BUFFER_(lexer)[i - 1])))
        --i;

    /* If the last trimmed space was escaped, then put it back. */

    if (i < lexer->pos_ && '\\' == BUFFER_(lexer)[i - 1])
        ++i;

    lexer->pos_ = i;
}

AUGUTIL_API aug_lexer_t
aug_createlexer(size_t size, aug_isdelim_t isdelim)
{
    aug_lexer_t lexer;

    /* Use default size when size has not been specified. */

    if (0 == size)
        size = AUG_BUFSIZE;

    if (!(lexer = malloc(sizeof(struct aug_lexer_) + size)))
        return NULL;

    lexer->pos_ = 0;
    lexer->size_ = size;
    lexer->state_ = LTRIM_;
    lexer->isdelim_ = isdelim;
    return lexer;
}

AUGUTIL_API int
aug_freelexer(aug_lexer_t lexer)
{
    free(lexer);
    return 0;
}

AUGUTIL_API enum aug_token
aug_lexchar(aug_lexer_t* lexer, char ch)
{
    char first;

    if ('\0' == ch)
        return -1 == aug_lexend(lexer) ? AUG_TOKERROR : AUG_TOKBREAK;

    switch ((*lexer)->state_) {
    case LTRIM_:

        /* Ignore non-eol whitespace while left-trimming. */

        if (islws_(ch))
            break;

        (*lexer)->state_ = DATA_;

        /* Fall-through */

    case DATA_:

        /* Check for match on specified delimiter character. */

        if ((*lexer)->isdelim_ && ((*lexer)->isdelim_)(ch)) {

            aug_lexend(lexer);
            return AUG_TOKDELIM;
        }

        switch (ch) {
        case '\r':
            rtrim_(*lexer);
            (*lexer)->state_ = CR_;
            break;

        case '\n':
            rtrim_(*lexer);
            (*lexer)->state_ = LF_;
            break;

        case '\\':
            (*lexer)->state_ = ESC_;

            /* Add back-slash to buffer: Fall-through. */

        default:
            if (-1 == append_(lexer, ch))
                return AUG_TOKERROR;
        }
        break;

    case ESC_:

        /* Escaped character is always appended. */

        if (-1 == append_(lexer, ch))
            return AUG_TOKERROR;

        (*lexer)->state_ = DATA_;
        break;

    case CR_:

        /* Solitary carriage-return characters, that are not part of a
           carriage-return/linefeed pair, are treated as errors: Not doing so
           would lead to ambiguities where a blank line is used to separate
           the header and body sections of a message. */

        if ('\n' != ch) {
            errno = EINVAL;
            return AUG_TOKERROR;
        }

        (*lexer)->state_ = LF_;
        break;

    case LF_:
        if (islws_(ch)) {

            /* Leading linear whitespace on a new line indicates a line
               continuation. */

            if (0 < (*lexer)->pos_ && -1 == append_(lexer, ' '))
                return AUG_TOKERROR;

            (*lexer)->state_ = LTRIM_;

        } else {

            /* Anything else indicates the start of a new line. */

            if (-1 == append_(lexer, '\0'))
                return AUG_TOKERROR;

            (*lexer)->pos_ = 0;

            if ('\n' == ch) {
                (*lexer)->state_ = LTRIM_;
                return AUG_TOKBREAK;
            }

            /* Note: The state is set to the first character of the new
               line. */

            (*lexer)->state_ = ch;
            return AUG_TOKLINE;
        }
        break;

    default: /* New line. */

        /* Parse the first two characters of the new line. */

        if ('\r' == (first = pending_(*lexer))) {

            if ('\n' != ch) {
                errno = EINVAL;
                return AUG_TOKERROR;
            }

            if (-1 == append_(lexer, '\0'))
                return AUG_TOKERROR;

            (*lexer)->pos_ = 0;
            (*lexer)->state_ = LTRIM_;
            return AUG_TOKBREAK;
        }

        switch (aug_lexchar(lexer, first)) {
        case AUG_TOKERROR:
            return AUG_TOKERROR;
        case AUG_TOKDELIM:
            (*lexer)->state_ = ch;
            return AUG_TOKDELIM;
        default:
            return aug_lexchar(lexer, ch);
        }
    }
    return AUG_TOKNONE;
}

AUGUTIL_API int
aug_lexend(aug_lexer_t* lexer)
{
    /* If present, add the first character of the new line. */

    if (0 < (*lexer)->state_
        && AUG_TOKERROR == aug_lexchar(lexer, pending_(*lexer)))
        return -1;

    rtrim_(*lexer);
    if (-1 == append_(lexer, '\0'))
        return -1;

    /* Reset. */

    (*lexer)->pos_ = 0;
    (*lexer)->state_ = LTRIM_;
    return 0;
}

AUGUTIL_API aug_isdelim_t
aug_setisdelim(aug_lexer_t lexer, aug_isdelim_t isdelim)
{
    int (*old)(char) = lexer->isdelim_;
    lexer->isdelim_ = isdelim;
    return old;
}

AUGUTIL_API const char*
aug_token(aug_lexer_t lexer)
{
    return BUFFER_(lexer);
}
