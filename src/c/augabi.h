/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
 * @defgroup Object Object
 */

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
        const struct name##vtbl* vtbl_;             \
        void* impl_;                                \
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

#define aug_cast(ob, type)                                      \
    ((aug_object*)(ob))->vtbl_->cast_((aug_object*)(ob), type)

#define aug_retain(ob)                                      \
    ((aug_object*)(ob))->vtbl_->retain_((aug_object*)(ob))

#define aug_release(ob)                                     \
    ((aug_object*)(ob))->vtbl_->release_((aug_object*)(ob))

/**
 * Assign pointer before calling release, avoiding potential reentrancy
 * issues.
 */

#define aug_safeassign(lhs, rhs)                        \
    do {                                                \
        aug_object* aug_tmp = (aug_object*)(lhs);       \
        (lhs) = (rhs);                                  \
        if (aug_tmp)                                    \
            aug_release(aug_tmp);                       \
    } while (0)

/** @} */

#endif /* AUGABI_H */
