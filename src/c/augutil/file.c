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
#define AUGUTIL_BUILD
#include "augutil/file.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/shellwords.h"
#include "augutil/xstr.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <assert.h>
#include <errno.h>

struct state_ {
    aug_confcb_t cb_;
    void* arg_;
    aug_xstr_t key_;
    aug_xstr_t value_;
    int last_;
    aug_result result_;
};

static aug_result
init_(struct state_* st, aug_confcb_t cb, void* arg)
{
    aug_mpool* mpool = aug_getmpool(aug_tlx);
    aug_xstr_t key, value;

    if (!(key = aug_createxstr(mpool, 64))) {
        aug_release(mpool);
        return -1;
    }

    value = aug_createxstr(mpool, 64);

    /* Done with mpool regardless. */

    aug_release(mpool);

    if (!value) {
        aug_destroyxstr(key);
        return -1;
    }

    /* Commit. */

    st->cb_ = cb;
    st->arg_ = arg;
    st->key_ = key;
    st->value_ = value;
    st->last_ = AUG_TOKPHRASE;
    st->result_ = 0;

    return 0;
}

static void
term_(struct state_* st)
{
    aug_destroyxstr(st->key_);
    aug_destroyxstr(st->value_);
}

static void
out_(void* arg, int what)
{
    struct state_* st = arg;
    aug_xstr_t tmp;

    /* Bail on error. */

    if (st->result_ < 0)
        return;

    switch (what) {
    case AUG_TOKERROR:
        /* Invalid case. */
        assert(0);
    case AUG_TOKPHRASE:

        /* Did this phrase contain anything significant? */

        if (AUG_TOKPHRASE != st->last_) {

            /* Key must not be empty. */

            if (0 != aug_xstrlen(st->key_)) {

                /* Callback with pair. */

                st->result_ = st->cb_(aug_xstr(st->key_),
                                      aug_xstr(st->value_), st->arg_);
                aug_clearxstr(st->key_);
                aug_clearxstr(st->value_);

            } else {

                aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug",
                                AUG_ENULL, AUG_MSG("empty key"));
                st->result_ = -1;
            }
        }
        break;
    case AUG_TOKLABEL:

        /* Rotate strings so that token becomes key. */

        tmp = st->key_;
        st->key_ = st->value_;
        st->value_ = tmp;

        break;
    case AUG_TOKWORD:
        break;
    case AUG_TOKRTRIM:
        /* Invalid case. */
        assert(0);
    default:

        /* Join next word. */

        if (AUG_TOKWORD == st->last_)
            aug_xstrcatc(st->value_, ' ');

        /* Append character. */

        aug_xstrcatc(st->value_, what);
        break;
    }
    st->last_ = what;
}

AUGUTIL_API aug_result
aug_freadconf(FILE* fp, aug_confcb_t cb, void* arg)
{
    struct state_ st;
    struct aug_words words;
    int ch;

    if (init_(&st, cb, arg) < 0)
        return -1;

    aug_initshellwords(&words, AUG_TRUE, out_, &st);

    while (EOF != (ch = fgetc(fp))) {

        aug_putshellwords(&words, ch);

        /* Check for errors during callback. */

        if (st.result_ < 0)
            goto done;
    }

    if (feof(fp))
        aug_putshellwords(&words, '\n');
    else {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        st.result_ = -1;
    }

 done:
    term_(&st);
    return st.result_;
}

AUGUTIL_API aug_result
aug_readconf(const char* path, aug_confcb_t cb, void* arg)
{
    FILE* fp = fopen(path, "r");
    aug_result result;

    if (fp) {
        result = aug_freadconf(fp, cb, arg);
        fclose(fp);
    } else {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        result = -1;
    }

    return result;
}
