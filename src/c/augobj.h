/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGOBJ_H
#define AUGOBJ_H

#include <stddef.h>
#include <sys/types.h> /* size_t */

/*
  The offsetof() macro can only be used for PODs.  GCC will emit the following
  warning when used on non-POD types:

  warning: (perhaps the `offsetof' macro was used incorrectly)
*/

#if !defined(offsetof)
# define offsetof(s, m) (size_t)&(((s*)0)->m)
#endif /* !offsetof */

#define AUG_EQUALID(a, b)                       \
    (0 == strcmp(a, b))

#define AUG_PODIMPL(s, m, ptr)                          \
    (ptr ? (s*)((char*)(ptr) - offsetof(s, m)) : NULL)

#if !defined(AUG_MKSTR)
# define AUG_MKSTR_(x) #x
# define AUG_MKSTR(x) AUG_MKSTR_(x)
#endif /* !AUG_MKSTR */

#define AUG_OBJECTDECL(type)                        \
    struct type##vtbl;                              \
    typedef struct type##_ {                        \
            const struct type##vtbl* vtbl_;         \
            void* impl_;                            \
    } type;                                         \
    static const char type##id[] = AUG_MKSTR(type)

#define AUG_OBJECT(type)                        \
    void* (*cast_)(type*, const char*);         \
    int (*incref_)(type*);                      \
    int (*decref_)(type*)

AUG_OBJECTDECL(aug_object);
struct aug_objectvtbl {
    AUG_OBJECT(aug_object);
};

#define aug_cast(ob, type)                                     \
    ((aug_object*)ob)->vtbl_->cast_((aug_object*)ob, type)

#define aug_incref(ob)                                     \
    ((aug_object*)ob)->vtbl_->incref_((aug_object*)ob)

#define aug_decref(ob)                                     \
    ((aug_object*)ob)->vtbl_->decref_((aug_object*)ob)

#endif /* AUGOBJ_H */
