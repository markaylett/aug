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
        return AUG_FAILERROR;
    }

    value = aug_createxstr(mpool, 64);

    /* Done with mpool regardless. */

    aug_release(mpool);

    if (!value) {
        aug_destroyxstr(key);
        return AUG_FAILERROR;
    }

    /* Commit. */

    st->cb_ = cb;
    st->arg_ = arg;
    st->key_ = key;
    st->value_ = value;
    st->last_ = AUG_TOKPHRASE;
    st->result_ = AUG_SUCCESS;

    return AUG_SUCCESS;
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

    if (AUG_ISFAIL(st->result_))
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

                st->result_ = st->cb_(st->arg_, aug_xstr(st->key_),
                                      aug_xstr(st->value_));
                aug_clearxstr(st->key_);
                aug_clearxstr(st->value_);

            } else {

                aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",
                               AUG_ENULL, AUG_MSG("empty key"));
                st->result_ = AUG_FAILERROR;
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

    aug_verify(init_(&st, cb, arg));

    aug_initshellwords(&words, AUG_TRUE, out_, &st);

    while (EOF != (ch = fgetc(fp))) {

        aug_putshellwords(&words, ch);

        /* Check for errors during callback. */

        if (AUG_ISFAIL(st.result_))
            goto done;
    }

    if (feof(fp))
        aug_putshellwords(&words, '\n');
    else /* Error. */
        st.result_ =
            aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

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
    } else
        result = aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return result;
}
