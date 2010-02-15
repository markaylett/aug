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
#include "augutil/object.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augext/var.h"

#include <string.h>

#if !defined(_WIN32)
# if HAVE_ALLOCA_H
#  include <alloca.h>
# endif /* HAVE_ALLOCA_H */
#else /* _WIN32 */
# include <malloc.h> /* alloca() */
#endif /* _WIN32 */

struct boxintimpl_ {
    aug_boxint boxint_;
    int refs_;
    aug_mpool* mpool_;
    void (*destroy_)(int);
    int i_;
};

static void*
castboxint_(aug_boxint* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_boxintid)) {
        aug_retain(ob);
        return ob;
    }
    return NULL;
}

static void
retainboxint_(aug_boxint* ob)
{
    struct boxintimpl_* impl = AUG_PODIMPL(struct boxintimpl_, boxint_, ob);
    ++impl->refs_;
}

static void
releaseboxint_(aug_boxint* ob)
{
    struct boxintimpl_* impl = AUG_PODIMPL(struct boxintimpl_, boxint_, ob);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        if (impl->destroy_)
            impl->destroy_(impl->i_);
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static int
unboxint_(aug_boxint* ob)
{
    struct boxintimpl_* impl = AUG_PODIMPL(struct boxintimpl_, boxint_, ob);
    return impl->i_;
}

static const struct aug_boxintvtbl boxintvtbl_ = {
    castboxint_,
    retainboxint_,
    releaseboxint_,
    unboxint_
};

struct boxptrimpl_ {
    aug_boxptr boxptr_;
    int refs_;
    aug_mpool* mpool_;
    void (*destroy_)(void*);
    void* p_;
};

static void*
castboxptr_(aug_boxptr* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_boxptrid)) {
        aug_retain(ob);
        return ob;
    }
    return NULL;
}

static void
retainboxptr_(aug_boxptr* ob)
{
    struct boxptrimpl_* impl = AUG_PODIMPL(struct boxptrimpl_, boxptr_, ob);
    ++impl->refs_;
}

static void
releaseboxptr_(aug_boxptr* ob)
{
    struct boxptrimpl_* impl = AUG_PODIMPL(struct boxptrimpl_, boxptr_, ob);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        if (impl->destroy_)
            impl->destroy_(impl->p_);
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static void*
unboxptr_(aug_boxptr* ob)
{
    struct boxptrimpl_* impl = AUG_PODIMPL(struct boxptrimpl_, boxptr_, ob);
    return impl->p_;
}

static const struct aug_boxptrvtbl boxptrvtbl_ = {
    castboxptr_,
    retainboxptr_,
    releaseboxptr_,
    unboxptr_
};


struct blobimpl_ {
    aug_blob blob_;
    int refs_;
    aug_mpool* mpool_;
    size_t size_;
    char buf_[1];
};

static void*
castblob_(aug_blob* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_blobid)) {
        aug_retain(ob);
        return ob;
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
        aug_mpool* mpool = impl->mpool_;
        aug_freemem(mpool, impl);
        aug_release(mpool);
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
        *size = impl->size_;
    return impl->buf_;
}

static size_t
getblobsize_(aug_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    return impl->size_;
}

static const struct aug_blobvtbl blobvtbl_ = {
    castblob_,
    retainblob_,
    releaseblob_,
    getblobtype_,
    getblobdata_,
    getblobsize_
};

AUGUTIL_API aug_boxint*
aug_createboxint(aug_mpool* mpool, int i, void (*destroy)(int))
{
    struct boxintimpl_* impl = aug_allocmem(mpool,
                                            sizeof(struct boxintimpl_));
    if (!impl)
        return NULL;

    impl->boxint_.vtbl_ = &boxintvtbl_;
    impl->boxint_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;

    impl->destroy_ = destroy;
    impl->i_ = i;

    aug_retain(mpool);
    return &impl->boxint_;
}

AUGUTIL_API int
aug_obtoi(aug_object* ob)
{
    int i;
    aug_boxint* tmp;
    if (ob && (tmp = aug_cast(ob, aug_boxintid))) {
        i = aug_unboxint(tmp);
        aug_release(tmp);
    } else
        i = 0;
    return i;
}

AUGUTIL_API aug_boxptr*
aug_createboxptr(aug_mpool* mpool, void* p, void (*destroy)(void*))
{
    struct boxptrimpl_* impl = aug_allocmem(mpool,
                                            sizeof(struct boxptrimpl_));
    if (!impl)
        return NULL;

    impl->boxptr_.vtbl_ = &boxptrvtbl_;
    impl->boxptr_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;

    impl->destroy_ = destroy;
    impl->p_ = p;

    aug_retain(mpool);
    return &impl->boxptr_;
}

AUGUTIL_API void*
aug_obtop(aug_object* ob)
{
    void* p;
    aug_boxptr* tmp;
    if (ob && (tmp = aug_cast(ob, aug_boxptrid))) {
        p = aug_unboxptr(tmp);
        aug_release(tmp);
    } else
        p = NULL;
    return p;
}

AUGUTIL_API aug_blob*
aug_createblob(aug_mpool* mpool, const void* buf, size_t len)
{
    struct blobimpl_* impl = aug_allocmem(mpool,
                                          sizeof(struct blobimpl_) + len);
    if (!impl)
        return NULL;

    impl->blob_.vtbl_ = &blobvtbl_;
    impl->refs_ = 1;
    impl->mpool_ = mpool;

    impl->size_ = len;
    memcpy(impl->buf_, buf, len);
    impl->buf_[len] = '\0'; /* Null terminator. */

    aug_retain(mpool);
    return &impl->blob_;
}

AUGUTIL_API aug_result
aug_vunpackargs(aug_array* array, const char* sig, va_list args)
{
    unsigned argc, i = 0;
    struct aug_var* unwind = NULL;
    const char* type = NULL;

    /* Unless the array is empty, create a cache of unpacked arguments so that
       they can be released on failure. */

    if (0 < (argc = aug_getarraysize(array)))
        unwind = alloca(sizeof(struct aug_var) * argc);

    /* For each argument in the array. */

    while (i < argc) {

        struct aug_var* var = &unwind[i];
        if (aug_isfail(aug_getarrayvalue(array, i, var)))
            goto fail;

        /* Now that this argument has been unpacked, ensure that it is unwound
           (released) on failure. */

        ++i;

        /* Verify type against signature. */

        switch (*sig++) {
        case '\0':

            /* End of signature, so too many arguments. */

            aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                           AUG_MSG("too many arguments"));
            goto fail;
        case 'v':

            /* No type verification. */

            *va_arg(args, struct aug_var*) = *var;
            break;
        case 'i':
            if (AUG_VTINT32 != var->type_) {
                type = "int32";
                goto fail;
            }
            *va_arg(args, int32_t*) = AUG_VARINT32(var);
            break;
        case 'l':
            if (AUG_VTINT64 != var->type_) {
                type = "int64";
                goto fail;
            }
            *va_arg(args, int64_t*) = AUG_VARINT64(var);
            break;
        case 'd':
            if (AUG_VTDOUBLE != var->type_) {
                type = "double";
                goto fail;
            }
            *va_arg(args, double*) = AUG_VARDOUBLE(var);
            break;
        case 'b':
            if (AUG_VTBOOL != var->type_) {
                type = "bool";
                goto fail;
            }
            *va_arg(args, aug_bool*) = AUG_VARBOOL(var);
            break;
        case 'O':
            if (AUG_VTOBJECT != var->type_) {
                type = "object";
                goto fail;
            }
            *va_arg(args, aug_object**) = AUG_VAROBJECT(var);
            break;
        case 'A':
            if (AUG_VTARRAY != var->type_) {
                type = "array";
                goto fail;
            }
            *va_arg(args, aug_array**) = AUG_VARARRAY(var);
            break;
        case 'B':
            if (AUG_VTBLOB != var->type_) {
                type = "blob";
                goto fail;
            }
            *va_arg(args, aug_blob**) = AUG_VARBLOB(var);
            break;
        case 'S':
            if (AUG_VTSTRING != var->type_) {
                type = "string";
                goto fail;
            }
            *va_arg(args, aug_blob**) = AUG_VARSTRING(var);
            break;
        }
    }

    /* Success if the end of the signature has been reached. */

    if ('\0' == *sig)
        return AUG_SUCCESS;

    /* Fallthrough: Not end of signature, so too few arguments. */

    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                   AUG_MSG("not enough arguments"));

 fail:

    /* Argument type error.  Type specified in switch. */

    if (type)
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("argument %d: expected %s"), i, type);

    if (unwind) {

        /* Unwind those that were unpacked. */

        unsigned j = 0;
        for (; j < i; ++j)
            AUG_RELEASEVAR(&unwind[j]);
    }

    return AUG_FAILERROR;
}

AUGUTIL_API aug_result
aug_unpackargs(aug_array* array, const char* sig, ...)
{
    aug_result result;
    va_list args;
    va_start(args, sig);
    result = aug_vunpackargs(array, sig, args);
    va_end(args);
    return result;
}
