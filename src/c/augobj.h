/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGOBJ_H
#define AUGOBJ_H

#include <stddef.h>
#include <sys/types.h> /* size_t */

/**
 * Calculate offset of member @a m in structure @a s.
 *
 * The offsetof() macro can only be used for PODs.  GCC will emit the
 * following warning when used on non-POD types:
 *
 * warning: (perhaps the 'offsetof' macro was used incorrectly)
 */

#if !defined(offsetof)
# define offsetof(s, m) (size_t)&(((s*)0)->m)
#endif /* !offsetof */

#if !defined(AUG_MKSTR)
# define AUG_MKSTR_(x) #x
# define AUG_MKSTR(x) AUG_MKSTR_(x)
#endif /* !AUG_MKSTR */

/**
 * @defgroup ObjectMacros Object Macros
 *
 * Macros to simplify object declarations in c.
 *
 * @{
 */

#define AUG_PODIMPL(s, m, ptr)                          \
    (ptr ? (s*)((char*)(ptr) - offsetof(s, m)) : NULL)

/**
 * Compares to object-ids @a a and @a b for equality.
 */

#define AUG_EQUALID(a, b)                       \
    (0 == strcmp(a, b))

#define AUG_OBJECTDECL(type)                        \
    struct type##vtbl;                              \
    typedef struct type##_ {                        \
            const struct type##vtbl* vtbl_;         \
            void* impl_;                            \
    } type;                                         \
    static const char type##id[] = AUG_MKSTR(type)

#define AUG_OBJECT(type)                        \
    void* (*cast_)(type*, const char*);         \
    int (*retain_)(type*);                      \
    int (*release_)(type*)

/** @} */

/**
 * @defgroup Object Object
 */

/**
 * @defgroup aug_object aug_object
 *
 * @ingroup Object
 *
 * All objects implement this base interface.
 *
 * @{
 */

AUG_OBJECTDECL(aug_object);
struct aug_objectvtbl {
    AUG_OBJECT(aug_object);
};

#define aug_cast(ob, type)                                     \
    ((aug_object*)ob)->vtbl_->cast_((aug_object*)ob, type)

#define aug_retain(ob)                                     \
    ((aug_object*)ob)->vtbl_->retain_((aug_object*)ob)

#define aug_release(ob)                                     \
    ((aug_object*)ob)->vtbl_->release_((aug_object*)ob)

/** @} */

#endif /* AUGOBJ_H */
