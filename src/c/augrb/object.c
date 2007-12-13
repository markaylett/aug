/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define MAUD_BUILD
#include "augrb/object.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

struct blobimpl_ {
    aug_blob blob_;
    augrb_blob rbblob_;
    unsigned refs_;
    VALUE rbobj_;
};

static void*
castblob_(aug_blob* obj, const char* id)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, obj);
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_blobid)) {
        aug_incref(&impl->blob_);
        return &impl->blob_;
    } else if (AUG_EQUALID(id, augrb_blobid)) {
        aug_incref(&impl->rbblob_);
        return &impl->rbblob_;
    }
    return NULL;
}

static int
increfblob_(aug_blob* obj)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, obj);
    ++impl->refs_;
    return 0;
}

static int
decrefblob_(aug_blob* obj)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, obj);
    if (0 == --impl->refs_) {
        impl->rbobj_ = Qnil; /* Belt and braces. */
        rb_gc_unregister_address(&impl->rbobj_);
        free(impl);
    }
    return 0;
}

static const void*
blobdata_(aug_blob* obj, size_t* size)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, obj);
    if (size)
        *size = RSTRING(impl->rbobj_)->len;
    return RSTRING(impl->rbobj_)->ptr;
}

static size_t
blobsize_(aug_blob* obj)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, obj);
    return RSTRING(impl->rbobj_)->len;
}

static const struct aug_blobvtbl blobvtbl_ = {
    castblob_,
    increfblob_,
    decrefblob_,
    blobdata_,
    blobsize_
};

static void*
castrbblob_(augrb_blob* obj, const char* id)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, rbblob_, obj);
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, augrb_blobid)) {
        aug_incref(&impl->rbblob_);
        return &impl->rbblob_;
    } else if (AUG_EQUALID(id, aug_blobid)) {
        aug_incref(&impl->blob_);
        return &impl->blob_;
    }
    return NULL;
}

static int
increfrbblob_(augrb_blob* obj)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, rbblob_, obj);
    ++impl->refs_;
    return 0;
}

static int
decrefrbblob_(augrb_blob* obj)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, rbblob_, obj);
    if (0 == --impl->refs_) {
        impl->rbobj_ = Qnil; /* Belt and braces. */
        rb_gc_unregister_address(&impl->rbobj_);
        free(impl);
    }
    return 0;
}

static VALUE
getrbblob_(augrb_blob* obj)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, rbblob_, obj);
    return impl->rbobj_;
}

static const struct augrb_blobvtbl rbblobvtbl_ = {
    castrbblob_,
    increfrbblob_,
    decrefrbblob_,
    getrbblob_
};

aug_blob*
augrb_createblob(VALUE rbobj)
{
    struct blobimpl_* impl = malloc(sizeof(struct blobimpl_));
    if (!impl)
        return NULL;

    if (!rbobj)
        rbobj = Qnil;

    impl->blob_.vtbl_ = &blobvtbl_;
    impl->blob_.impl_ = NULL;

    impl->rbblob_.vtbl_ = &rbblobvtbl_;
    impl->rbblob_.impl_ = NULL;

    impl->refs_ = 1;
    impl->rbobj_ = rbobj;

    /* Prevent garbage collection. */

    rb_gc_register_address(&impl->rbobj_);
    return &impl->blob_;
}

const void*
augrb_blobdata(aug_object* obj, size_t* size)
{
    const void* data = NULL;
    if (obj) {
        aug_blob* blob = aug_cast(obj, aug_blobid);
        if (blob) {
            data = aug_blobdata(blob, size);
            aug_decref(blob);
        }
    }
    return data;
}

VALUE
augrb_getblob(aug_object* obj)
{
    VALUE rbobj = Qnil;
    if (obj) {
        augrb_blob* blob = aug_cast(obj, augrb_blobid);
        if (blob) {
            rbobj = blob->vtbl_->get_(blob);
            aug_decref(blob);
        }
    }
    return rbobj;
}
