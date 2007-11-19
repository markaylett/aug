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

typedef struct aug_object* aug_object_t;

struct aug_objectvtbl {
    AUG_OBJECT(aug_object);
};

struct aug_object {
    const struct aug_objectvtbl* vtbl_;
};

#define aug_castobject(obj, type)               \
    (obj)->vtbl_->cast_(obj, type)

#define aug_retainobject(obj)                   \
    (obj)->vtbl_->retain_(obj)

#define aug_releaseobject(obj)                  \
    (obj)->vtbl_->release_(obj)

typedef struct aug_blob* aug_blob_t;

struct aug_blobvtbl {
    AUG_OBJECT(aug_blob);
    const void* (*data_)(aug_blob_t, size_t*);
};

struct aug_blob {
    const struct aug_blobvtbl* vtbl_;
};

#define aug_castblob(obj, type)                 \
    (obj)->vtbl_->cast_(obj, type)

#define aug_retainblob(obj)                     \
    (obj)->vtbl_->retain_(obj)

#define aug_releaseblob(obj)                    \
    (obj)->vtbl_->release_(obj)

#define aug_blobdata(obj, size)                 \
    (obj)->vtbl_->data_(obj, size)

#endif /* AUGOBJ_H */
