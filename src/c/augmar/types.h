/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
 * @file types.h
 *
 * External type definitions.
 */

#ifndef AUGMAR_TYPES_H
#define AUGMAR_TYPES_H

#include "augsys/types.h"

#include <fcntl.h>
#include <stdarg.h>

/**
 * The message archive handle type.
 */

typedef struct aug_mar_* aug_mar_t;

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
 * Return code used by some functions to indicate that no matching item could
 * be found.
 */
#define AUG_RETNOMATCH (-2)

/**
 * @defgroup OpenFlags Open Flags
 * @{
 */

/**
 * Open for reading only.
 */
#define AUG_RDONLY O_RDONLY

/**
 * Open for writing only.
 */
#define AUG_WRONLY O_WRONLY

/**
 * Open for reading and writing.
 */
#define AUG_RDWR   O_RDWR

/**
 * Append on each write.
 */
#define AUG_APPEND O_APPEND

/**
 * Create file if it does not exist.
 */
#define AUG_CREAT  O_CREAT

/**
 * Truncate size to zero.
 */
#define AUG_TRUNC  O_TRUNC

/**
 * Error if create and file exists.
 */
#define AUG_EXCL   O_EXCL
/**
 * @}
 */

/**
 * @defgroup SeekWhence Seek Whence
 * @{
 */

/**
 * The offset is set to offset bytes.
 */
#define AUG_SET 0

/**
 * The offset is set to its current location plus offset bytes.
 */
#define AUG_CUR 1

/**
 * The offset is set to the size of the file plus offset bytes.
 */
#define AUG_END 2
/**
 * @}
 */

#endif /* AUGMAR_TYPES_H */
