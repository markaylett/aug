/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGTYPES_H
#define AUGTYPES_H

/**
 * Success and failure function return codes.  Akin to #EXIT_SUCCESS and
 * #EXIT_FAILURE.
 */

#define AUG_SUCCESS   0
#define AUG_FAILURE (-1)

/**
 * When returned as status codes, the negated form of the following exception
 * codes should be used.
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
 * End-of sequence.
 */

#define AUG_EENDOF    6

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
 * Invalid state transition.
 */

#define AUG_ESTATE   18

/**
 * Operation not supported.
 */

#define AUG_ESUPPORT 19

/**
 * Type-related error.
 */

#define AUG_ETYPE    20

/**
 * Timeout.
 */

#define AUG_ETIMEOUT 21

/* C++ enums are not guaranteed to have sizeof(int).  Therefore, int is used
   instead. */

enum aug_bool_ {
    AUG_FALSE,
    AUG_TRUE
};

typedef int aug_bool;

/**
 * Standard result type.
 *
 * Used to convey the following conventions: less than zero for errors; zero
 * for success; greater than zero for success with info.
 */

typedef int aug_result;

#endif /* AUGTYPES_H */
