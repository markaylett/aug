/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUB_H
#define AUB_H

/**
 * @page aub
 *
 * TODO.
 */

/**
 * @page aubidl
 *
 * TODO.
 */

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

#define AUB_MKSTR_(x) #x
#define AUB_MKSTR(x) AUB_MKSTR_(x)

/**
 * @defgroup ObjectMacros Object Macros
 *
 * Macros to simplify object declarations in c.
 *
 * @{
 */

#define AUB_PODIMPL(s, m, ptr)                          \
    (ptr ? (s*)((char*)(ptr) - offsetof(s, m)) : NULL)

/**
 * Compares to object-ids @a a and @a b for equality.
 */

#define AUB_EQUALID(a, b)                       \
    (0 == strcmp(a, b))

#define AUB_OBJECTDECL(type)                        \
    struct type##vtbl;                              \
    typedef struct type##_ {                        \
            const struct type##vtbl* vtbl_;         \
            void* impl_;                            \
    } type;                                         \
    static const char type##id[] = AUB_MKSTR(type)

#define AUB_OBJECT(type)                        \
    void* (*cast_)(type*, const char*);         \
    int (*retain_)(type*);                      \
    int (*release_)(type*)

/** @} */

/**
 * @defgroup Object Object
 */

/**
 * @defgroup aub_object aub_object
 *
 * @ingroup Object
 *
 * All objects implement this base interface.
 *
 * @{
 */

AUB_OBJECTDECL(aub_object);
struct aub_objectvtbl {
    AUB_OBJECT(aub_object);
};

#define aub_cast(ob, type)                                     \
    ((aub_object*)ob)->vtbl_->cast_((aub_object*)ob, type)

#define aub_retain(ob)                                     \
    ((aub_object*)ob)->vtbl_->retain_((aub_object*)ob)

#define aub_release(ob)                                     \
    ((aub_object*)ob)->vtbl_->release_((aub_object*)ob)

/** @} */

#endif /* AUB_H */
