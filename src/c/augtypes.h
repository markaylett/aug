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
#ifndef AUGTYPES_H
#define AUGTYPES_H

#include "augconfig.h"

/**
 * @defgroup Types Types
 */

/**
 * @defgroup TypesBool Boolean Values
 *
 * @ingroup Types
 *
 * @{
 */

/**
 * Boolean type.
 *
 * C++ enums are not guaranteed to have sizeof(int) so int is used instead.
 *
 * Do not use #AUG_TRUE in tests as any non-zero value is considered true.
 *
 */

typedef int aug_bool;

#define AUG_FALSE 0
#define AUG_TRUE  1

/** @} */

/**
 * Descriptor type.  Negative values may have special meanings, such as -1 to
 * mean bad descriptor.
 */

typedef int aug_id;

/**
 * Equivalent to socklen_t.
 */

typedef unsigned aug_len;

/**
 * Equivalent to suseconds_t.
 */

typedef long aug_suseconds;

/**
 * Equivalent to time_t.
 */

typedef long aug_time;

/**
 * Portable timeval definition to ensure standard layout across compilers.
 * The original member names have been maintained to allow use with existing
 * macros.
 */

struct aug_timeval {
    /**
     * Whole seconds.
     */
    aug_time tv_sec;
    /**
     * Fractional seconds to microsecond accuracy.
     */
    aug_suseconds tv_usec;
};

/**
 * The field structure.
 */

struct aug_field {
    /**
     * The field name.
     */
    const char* name_;
    /**
     * A pointer to the field's value.
     */
    const void* value_;
    /**
     * The size of the field's value.
     */
    unsigned size_;
};

/**
 * @defgroup TypesResult Result Codes
 *
 * @ingroup Types
 *
 * Standard integer return codes.
 *
 * @{
 */

/**
 * Standard result type.
 *
 * The @ref aug_result type should be used instead of int, where functions
 * return 0 or -1 to indicate success or failure.  The type helps to
 * communicate intent.
 *
 * Negative aug_result values indicate exceptions.  Not all exceptions are
 * errors.  The posix errno values @ref EINTR and @ref EWOULDBLOCK are good
 * examples of this.  Equivalents of these are @ref AUG_FAILINTR and @ref
 * AUG_FAILBLOCK.
 */

# if HAVE_SYS_TYPES_H
#  include <sys/types.h> /* ssize_t */
# endif /* HAVE_SYS_TYPES_H */

typedef int aug_result;

typedef int aug_rint;
typedef long aug_rlong;
typedef ssize_t aug_rsize;

/** @} */

/**
 * @defgroup TypesException Exception Codes
 *
 * @ingroup Types
 *
 * Standard exceptions.  aug_clearerrinfo() should be called before returning
 * these.
 *
 * @{
 */

#define AUG_EXERROR 0x01

/**
 * Empty or missing entry.
 */

#define AUG_EXNONE 0x02 /* ENOENT */

/**
 * Interrupted system call.
 */

#define AUG_EXINTR  0x04 /* EINTR, WSAEINTR */

/**
 * Blocked, busy or pending.
 */

#define AUG_EXBLOCK 0x08 /* EAGAIN, EWOULDBLOCK, WSAEWOULDBLOCK */

#define AUG_EXALL   (AUG_EXERROR | AUG_EXNONE | AUG_EXINTR | AUG_EXBLOCK)

/** @} */

/**
 * @defgroup TypesError Error Codes
 *
 * @ingroup Types
 *
 * When returned as status codes, the negated form of the following exception
 * codes should be used.
 *
 * @{
 */

/**
 * Assertion failure.
 */

#define AUG_EASSERT   2

/**
 * Authentication failure.
 */

#define AUG_EAUTH     3

/**
 * Configuration error.
 */

#define AUG_ECONFIG   4

/**
 * Domain-level error.
 */

#define AUG_EDOMAIN   5

/**
 * C++ exception.
 */

#define AUG_EEXCEPT   7

/**
 * Either exists or does not exist.
 */

#define AUG_EEXIST    8

/**
 * Formatting error.
 */

#define AUG_EFORMAT   9

/**
 * Invalid argument.
 */

#define AUG_EINVAL   10

/**
 * IO error.
 */

#define AUG_EIO      11

/**
 * Limit exceeded.
 */

#define AUG_ELIMIT   12

/**
 * Null pointer.
 */

#define AUG_ENULL    13

/**
 * Out of memory.
 */

#define AUG_EMEMORY  14

/**
 * Parse error.
 */

#define AUG_EPARSE   15

/**
 * Permission denied.
 */

#define AUG_EPERM    16

/**
 * Range or bounds exceeded.
 */

#define AUG_ERANGE   17

/**
 * Illegal or invalid state.
 */

#define AUG_ESTATE   18

/**
 * Operation not supported.
 */

#define AUG_ESUPPORT 19

/**
 * Dynamic type error.
 */

#define AUG_ETYPE    20

/** @} */

#endif /* AUGTYPES_H */
