/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/networds.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

/*
  Fold Word Semantics

  message         =   *field *(CRLF *text)

  field           =    field-name ":" [field-body] CRLF

  field-name      =  1*<any CHAR, excluding CTLs, SPACE, and ":">

  field-body      =   *text [CRLF LWSP-char field-body]

  Headers occur before the message body and are terminated by a null line
  (i.e., two contiguous CRLFs).

  A line which continues a header field begins with a SPACE or HTAB character,
  while a line beginning a field starts with a printable character which is
  not a colon.

  A field-name consists of one or more printable characters (excluding colon,
  space, and control-characters).  A field-name MUST be contained on one line.
  Upper and lower case are not distinguished when comparing field-names.
*/

#include <ctype.h>
#include <stdio.h>

static void
eatws_(struct aug_words*, int);

static void
word_(struct aug_words*, int);

static void
eatws_(struct aug_words* st, int ch)
{
    if (st->flags_ & AUG_WRDESCAPE) {
        /* Handle escape. */
        st->flags_ &= ~AUG_WRDESCAPE;
        /* Begin word with literal. */
        st->out_(st->arg_, ch);
        st->fn_ = word_;
    } else {
        switch (ch) {
        case '\n':  /* New line. */
            if (!(st->flags_ & AUG_WRDLABEL)) {
                /* Label seen so possible continuation. */
                st->fn_ = word_;
                st->fn_(st, ch);
            }
            break;
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':  /* Line space. */
            /* Swallow. */
            break;
        case '\\': /* Escape. */
            /* Begin escape. */
            st->flags_ |= AUG_WRDESCAPE;
            break;
        default:   /* Else. */
            /* Begin word. */
            st->out_(st->arg_, ch);
            st->fn_ = word_;
            break;
        }
    }
}

static void
word_(struct aug_words* st, int ch)
{
    if (st->flags_ & AUG_WRDNEWLINE) {
        /* Handle newline. */
        st->flags_ &= ~AUG_WRDNEWLINE;
        switch (ch) {
        case '\n':  /* New line. */
            /* Two new lines. */
            st->out_(st->arg_, AUG_TOKWORD);
            st->out_(st->arg_, AUG_TOKPHRASE);
            /* Begin new phrase. */
            st->fn_ = eatws_;
            st->flags_ = AUG_WRDLABEL;
            break;
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':  /* Line space. */
            /* Join split lines with a space. */
            st->out_(st->arg_, ' ');
            st->fn_ = eatws_;
            break;
        case '\\': /* Escape. */
            /* Begin new word. */
            st->out_(st->arg_, AUG_TOKWORD);
            st->flags_ |= (AUG_WRDESCAPE | AUG_WRDLABEL);
            break;
        default:
            st->out_(st->arg_, AUG_TOKWORD);
            /* Append to new word. */
            st->out_(st->arg_, ch);
            st->flags_ |= AUG_WRDLABEL;
            break;
        }
    } else if (st->flags_ & AUG_WRDESCAPE) {
        /* Handle escape. */
        st->flags_ &= ~AUG_WRDESCAPE;
        /* Literal. */
        st->out_(st->arg_, ch);
    } else {
        switch (ch) {
        case '\n':  /* New line. */
            /* Trim trailing whitespace. */
            st->out_(st->arg_, AUG_TOKRTRIM);
            /* Begin newline. */
            st->flags_ |= AUG_WRDNEWLINE;
            break;
        case '\\': /* Escape. */
            /* Begin escape. */
            st->flags_ |= AUG_WRDESCAPE;
            break;
        default:   /* Else. */
            if ((st->flags_ & AUG_WRDLABEL) && ':' == ch) {
                /* Trim and label. */
                st->out_(st->arg_, AUG_TOKRTRIM);
                st->out_(st->arg_, AUG_TOKLABEL);
                st->fn_ = eatws_;
                st->flags_ &= ~AUG_WRDLABEL;
            } else {
                /* Append to word. */
                st->out_(st->arg_, ch);
            }
            break;
        }
    }
}

AUGUTIL_API void
aug_initnetwords(struct aug_words* st, void (*out)(void*, int), void* arg)
{
    st->out_ = out;
    st->arg_ = arg;
    st->fn_ = eatws_;
    st->flags_ = AUG_WRDLABEL;
}

AUGUTIL_API void
aug_putnetwords(struct aug_words* st, int ch)
{
    switch (ch) {
    case EOF:
        st->fn_(st, '\n');
        break;
    case '\r':
        /* Ignore. */
        break;
    default:
        st->fn_(st, ch);
        break;
    }
}

AUGUTIL_API size_t
aug_rtrimword(const char* src, size_t size)
{
    /* Find last non-space character, or beginning of buffer if none exist. */

    size_t i = size;
    while (0 < i && isspace((int)src[i - 1]))
        --i;

    /* If the last trimmed space was escaped, then put it back. */

    if (i < size && '\\' == src[i - 1])
        ++i;

    return i;
}
