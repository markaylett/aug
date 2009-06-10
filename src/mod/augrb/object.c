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
#include "object.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

struct boximpl_ {
    augrb_box box_;
    int refs_;
    VALUE rbob_;
};

static void*
castbox_(augrb_box* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, augrb_boxid)) {
        aug_retain(ob);
        return ob;
    }
    return NULL;
}

static void
retainbox_(augrb_box* ob)
{
    struct boximpl_* impl = AUG_PODIMPL(struct boximpl_, box_, ob);
    ++impl->refs_;
}

static void
releasebox_(augrb_box* ob)
{
    struct boximpl_* impl = AUG_PODIMPL(struct boximpl_, box_, ob);
    if (0 == --impl->refs_) {
        impl->rbob_ = Qnil; /* Belt and braces. */
        rb_gc_unregister_address(&impl->rbob_);
        free(impl);
    }
}

static VALUE
unbox_(augrb_box* ob)
{
    struct boximpl_* impl = AUG_PODIMPL(struct boximpl_, box_, ob);
    return impl->rbob_;
}

static const struct augrb_boxvtbl boxvtbl_ = {
    castbox_,
    retainbox_,
    releasebox_,
    unbox_
};

augrb_box*
augrb_createbox(VALUE rbob)
{
    struct boximpl_* impl = malloc(sizeof(struct boximpl_));
    if (!impl)
        return NULL;

    if (!rbob)
        rbob = Qnil;

    impl->box_.vtbl_ = &boxvtbl_;
    impl->box_.impl_ = NULL;

    impl->refs_ = 1;
    impl->rbob_ = rbob;

    /* Prevent garbage collection. */

    rb_gc_register_address(&impl->rbob_);
    return &impl->box_;
}

struct blobimpl_ {
    aug_blob blob_;
    augrb_box box_;
    int refs_;
    VALUE rbob_;
};

static void*
castblob_(aug_blob* ob, const char* id)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_blobid)) {
        aug_retain(&impl->blob_);
        return &impl->blob_;
    } else if (AUG_EQUALID(id, augrb_boxid)) {
        aug_retain(&impl->box_);
        return &impl->box_;
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

static const char*
getblobtype_(aug_blob* ob)
{
    return "application/octet-stream";
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
    getblobtype_,
    getblobdata_,
    getblobsize_
};

static void*
castboxblob_(augrb_box* ob, const char* id)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, box_, ob);
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, augrb_boxid)) {
        aug_retain(&impl->box_);
        return &impl->box_;
    } else if (AUG_EQUALID(id, aug_blobid)) {
        aug_retain(&impl->blob_);
        return &impl->blob_;
    }
    return NULL;
}

static void
retainboxblob_(augrb_box* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, box_, ob);
    ++impl->refs_;
}

static void
releaseboxblob_(augrb_box* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, box_, ob);
    if (0 == --impl->refs_) {
        impl->rbob_ = Qnil; /* Belt and braces. */
        rb_gc_unregister_address(&impl->rbob_);
        free(impl);
    }
}

static VALUE
unboxblob_(augrb_box* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, box_, ob);
    return impl->rbob_;
}

static const struct augrb_boxvtbl boxblobvtbl_ = {
    castboxblob_,
    retainboxblob_,
    releaseboxblob_,
    unboxblob_
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

    impl->box_.vtbl_ = &boxblobvtbl_;
    impl->box_.impl_ = NULL;

    impl->refs_ = 1;
    impl->rbob_ = rbob;

    /* Prevent garbage collection. */

    rb_gc_register_address(&impl->rbob_);
    return &impl->blob_;
}

VALUE
augrb_obtorb(aug_object* ob)
{
    VALUE rbob = Qnil;
    if (ob) {
        augrb_box* blob = aug_cast(ob, augrb_boxid);
        if (blob) {
            rbob = blob->vtbl_->unbox_(blob);
            aug_release(blob);
        }
    }
    return rbob;
}
