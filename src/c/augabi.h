/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGABI_H
#define AUGABI_H

/**
 * @file augabi.h
 *
 * Application binary interface.
 *
 * Binary layout for component interfaces.  Components that adhere to this
 * specification may safely cross C/C++ language, and compiler boundaries.
 */

/**
 * @page augidl
 *
 * Tool for creating interfaces from XML.
 */

#include <stddef.h>
#include <sys/types.h> /* size_t */

#if !defined(__cplusplus)
# define AUG_EXTERNC extern
#else /* __cplusplus */
# define AUG_EXTERNC extern "C"
#endif /* __cplusplus */

#if defined(__CYGWIN__) || defined(__MINGW32__)
# define AUG_EXPORT __attribute__ ((dllexport))
# define AUG_IMPORT __attribute__ ((dllimport))
#elif defined(_MSC_VER)
# define AUG_EXPORT __declspec(dllexport)
# define AUG_IMPORT __declspec(dllimport)
#else /* !__CYGWIN__ && !__MINGW__ && !__MSC_VER */
# define AUG_EXPORT
# define AUG_IMPORT
#endif /* !__CYGWIN__ && !__MINGW__ && !__MSC_VER */

/**
 * @defgroup ObjectMacros Macros
 *
 * @ingroup Object
 *
 * Macros to simplify object declarations in c.
 *
 * @{
 */

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

/** @{ */

/**
 * Stringify.
 */

#define AUG_MKSTR_(x) #x
#define AUG_MKSTR(x) AUG_MKSTR_(x)

/** @} */

/**
 * Get the containing object of type @a s, with interface member @a m, from
 * interface pointer @a ptr,
 */

#define AUG_PODIMPL(s, m, ptr)                          \
    (ptr ? (s*)((char*)(ptr) - offsetof(s, m)) : NULL)

/**
 * Compares to object-ids @a a and @a b for equality.
 */

#define AUG_EQUALID(a, b)                       \
    (0 == strcmp(a, b))

/**
 * Declare an interface called @a name.
 */

#define AUG_INTERFACE(name)                         \
    struct name##vtbl;                              \
    typedef struct name##_ {                        \
            const struct name##vtbl* vtbl_;         \
            void* impl_;                            \
    } name;                                         \
    static const char name##id[] = AUG_MKSTR(name)

/**
 * Declare base members of vtable for interface @a iface.
 */

#define AUG_VTBL(iface)                          \
    void* (*cast_)(iface*, const char*);         \
    void (*retain_)(iface*);                     \
    void (*release_)(iface*)

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

AUG_INTERFACE(aug_object);

struct aug_objectvtbl {
    AUG_VTBL(aug_object);
};

#define aug_cast(ob, type)                                     \
    ((aug_object*)ob)->vtbl_->cast_((aug_object*)ob, type)

#define aug_retain(ob)                                     \
    ((aug_object*)ob)->vtbl_->retain_((aug_object*)ob)

#define aug_release(ob)                                     \
    ((aug_object*)ob)->vtbl_->release_((aug_object*)ob)

/** @} */

#endif /* AUGABI_H */
