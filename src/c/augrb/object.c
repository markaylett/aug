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
#define MOD_BUILD
#include "augrb/object.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

struct blobimpl_ {
    aug_blob blob_;
    augrb_blob rbblob_;
    unsigned refs_;
    VALUE rbob_;
};

static void*
castblob_(aug_blob* ob, const char* id)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_blobid)) {
        aug_retain(&impl->blob_);
        return &impl->blob_;
    } else if (AUG_EQUALID(id, augrb_blobid)) {
        aug_retain(&impl->rbblob_);
        return &impl->rbblob_;
    }
    return NULL;
}

static void
retainblob_(aug_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    ++impl->refs_;
}

static void
releaseblob_(aug_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    if (0 == --impl->refs_) {
        impl->rbob_ = Qnil; /* Belt and braces. */
        rb_gc_unregister_address(&impl->rbob_);
        free(impl);
    }
}

static const void*
getblobdata_(aug_blob* ob, size_t* size)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    if (size)
        *size = RSTRING(impl->rbob_)->len;
    return RSTRING(impl->rbob_)->ptr;
}

static size_t
getblobsize_(aug_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    return RSTRING(impl->rbob_)->len;
}

static const struct aug_blobvtbl blobvtbl_ = {
    castblob_,
    retainblob_,
    releaseblob_,
    getblobdata_,
    getblobsize_
};

static void*
castrbblob_(augrb_blob* ob, const char* id)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, rbblob_, ob);
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, augrb_blobid)) {
        aug_retain(&impl->rbblob_);
        return &impl->rbblob_;
    } else if (AUG_EQUALID(id, aug_blobid)) {
        aug_retain(&impl->blob_);
        return &impl->blob_;
    }
    return NULL;
}

static void
retainrbblob_(augrb_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, rbblob_, ob);
    ++impl->refs_;
}

static void
releaserbblob_(augrb_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, rbblob_, ob);
    if (0 == --impl->refs_) {
        impl->rbob_ = Qnil; /* Belt and braces. */
        rb_gc_unregister_address(&impl->rbob_);
        free(impl);
    }
}

static VALUE
getrbblob_(augrb_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, rbblob_, ob);
    return impl->rbob_;
}

static const struct augrb_blobvtbl rbblobvtbl_ = {
    castrbblob_,
    retainrbblob_,
    releaserbblob_,
    getrbblob_
};

aug_blob*
augrb_createblob(VALUE rbob)
{
    struct blobimpl_* impl = malloc(sizeof(struct blobimpl_));
    if (!impl)
        return NULL;

    if (!rbob)
        rbob = Qnil;

    impl->blob_.vtbl_ = &blobvtbl_;
    impl->blob_.impl_ = NULL;

    impl->rbblob_.vtbl_ = &rbblobvtbl_;
    impl->rbblob_.impl_ = NULL;

    impl->refs_ = 1;
    impl->rbob_ = rbob;

    /* Prevent garbage collection. */

    rb_gc_register_address(&impl->rbob_);
    return &impl->blob_;
}

VALUE
augrb_getblob(aug_object* ob)
{
    VALUE rbob = Qnil;
    if (ob) {
        augrb_blob* blob = aug_cast(ob, augrb_blobid);
        if (blob) {
            rbob = blob->vtbl_->get_(blob);
            aug_release(blob);
        }
    }
    return rbob;
}
