/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGOBJ_H
#define AUGOBJ_H

#include <stddef.h>
#include <sys/types.h> /* size_t */

#if !defined(offsetof)
# define offsetof(s, m) (size_t)&(((s*)0)->m)
#endif /* !offsetof */

#define AUG_IMPL(s, m, ptr)                             \
    (ptr ? (s*)((char*)(ptr) - offsetof(s, m)) : NULL)

#define AUG_OBJECT(type)                        \
    void* (*cast_)(struct type*, const char*);  \
    int (*retain_)(struct type*);               \
    int (*release_)(struct type*)

struct aug_objectvtbl;

typedef struct aug_object {
    const struct aug_objectvtbl* vtbl_;
}* aug_object_t;

#define aug_objecttype() "aug_object"

struct aug_objectvtbl {
    AUG_OBJECT(aug_object);
};

#define aug_castobject(obj, type)                           \
    ((aug_object_t)(obj))->vtbl_->cast_((void*)obj, type)

#define aug_retainobject(obj)                           \
    ((aug_object_t)(obj))->vtbl_->retain_((void*)obj)

#define aug_releaseobject(obj)                          \
    ((aug_object_t)(obj))->vtbl_->release_((void*)obj)

struct aug_blobvtbl;

typedef struct aug_blob {
    const struct aug_blobvtbl* vtbl_;
}* aug_blob_t;

#define aug_blobtype() "aug_blob"

struct aug_blobvtbl {
    AUG_OBJECT(aug_blob);
    const void* (*data_)(aug_blob_t, size_t*);
};

#define aug_blobdata(obj, size)                 \
    (obj)->vtbl_->data_(obj, size)

#endif /* AUGOBJ_H */
