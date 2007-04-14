/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

/**
   \file mar.h
   \brief The MAR API header file.
*/

#ifndef AUGMAR_MAR_H
#define AUGMAR_MAR_H

#include "augmar/config.h"
#include "augmar/types.h"

/**
   \brief Copy an existing message archive.

   \param dst A handle to the destination message archive.

   \param src A handle to the source message archive.

   \return Zero on success or -1 on failure, in which case errno can be used
   to determine the error.

   \sa aug_retainmar().
*/

AUGMAR_API int
aug_copymar(aug_mar_t dst, aug_mar_t src);

/**
   \brief Create an in-memory message archive.

   \return A handle to the newly created message archive or NULL on failure,
   in which case errno can be used to determine the error.

   \sa aug_openmar() and aug_releasemar().
*/

AUGMAR_API aug_mar_t
aug_createmar(void);

/**
   \brief Create or open a file-based message archive.

   \param path A path to the file to be created or opened.

   \param flags The \ref OpenFlags "flags" used to create or open the file.

   \param ... The permissions used to create a new file.  Required when the
   #AUG_CREAT flag has been specified, otherwise ignored.

   \return A handle to the newly created message archive or NULL on failure,
   in which case errno can be used to determine the error.

   \sa aug_createmar() and aug_releasemar().
*/

AUGMAR_API aug_mar_t
aug_openmar(const char* path, int flags, ...);

/**
   \brief Release message archive handle.

   This function must be used to release handles allocated by either the
   aug_createmar() or aug_openmar() functions.

   \param mar A handle to the message archive to be released.

   \return Zero on success or -1 on failure, in which case errno can be used
   to determine the error.

   \sa aug_createmar(), aug_openmar() and aug_retainmar().
*/

AUGMAR_API int
aug_releasemar(aug_mar_t mar);

/**
   \brief Retain additional reference to message archive handle.

   \param mar A handle to the message archive.

   \return Zero on success or -1 on failure, in which case errno can be used
   to determine the error.

   \sa aug_copymar() and aug_releasemar().
*/

AUGMAR_API int
aug_retainmar(aug_mar_t mar);

/**
   \brief Compact any unused space within message archive.

   \param mar A handle to the message archive.

   \return ero on success or -1 on failure, in which case errno can be used
   to determine the error.

   \sa aug_truncatemar().
*/

AUGMAR_API int
aug_compactmar(aug_mar_t mar);

/**
   \brief Remove all fields contained within message archive.

   \param mar A handle to the message archive.

   \return Zero on success or -1 on failure, in which case errno can be used
   to determine the error.

   \sa aug_unsetbyname() and aug_unsetbyord().
*/

AUGMAR_API int
aug_removefields(aug_mar_t mar);

/**
   \brief Set field within message archive.

   \param mar A handle to the message archive.

   \param field A field specifying the field name and a pointer to, and size
   of, the field value.

   \param ord An optional output parameter, in which, the ordinal position of
   the field will be set.

   \return Zero on success or -1 on failure, in which case errno can be used
   to determine the error.

   \sa aug_setvalue().
*/

AUGMAR_API int
aug_setfield(aug_mar_t mar, const struct aug_field* field, unsigned* ord);

/**
   \brief Set field value within message archive.

   \param mar A handle to the message archive.

   \param ord The ordinal position of an existing field.

   \param value A pointer to the field value to be assigned.

   \param size The size of the field value to be assigned.

   \return Zero on success or -1 on failure, in which case errno can be used
   to determine the error.

   \sa aug_setfield().
*/

AUGMAR_API int
aug_setvalue(aug_mar_t mar, unsigned ord, const void* value, unsigned size);

/**
   \brief Unset field (by name) within message archive.

   \param mar A handle to the message archive.

   \param name The name of the field.

   \param ord An optional output parameter, in which, the ordinal position of
   the field is set.

   \return Zero on success, #AUG_RETNOMATCH if there is no matching field, or
   -1 on failure, in which case errno can be used to determine the error.

   \sa aug_removefields(), aug_unsetbyord() and aug_ordtoname().
*/

AUGMAR_API int
aug_unsetbyname(aug_mar_t mar, const char* name, unsigned* ord);

/**
   \brief Unset field (by ordinal) within message archive.

   \param mar A handle to the message archive.

   \param ord The ordinal position of the field.

   \return Zero on success, #AUG_RETNOMATCH if there is no matching field, or
   -1 on failure, in which case errno can be used to determine the error.

   \sa aug_removefields(), aug_unsetbyname() and aug_nametoord().
*/

AUGMAR_API int
aug_unsetbyord(aug_mar_t mar, unsigned ord);

/**
   \brief Obtain field value (by name) from message archive.

   \param mar A handle to the message archive.

   \param name The name of the field.

   \param size An optional output parameter, in which, the size of the field
   value is set.

   \return A pointer to the field value or NULL on failure, in which case
   errno can be used to determine the error.

   \sa aug_valuebyord(), aug_getfield() and aug_ordtoname().
*/

AUGMAR_API const void*
aug_valuebyname(aug_mar_t mar, const char* name, unsigned* size);

/**
   \brief Obtain field value (by ordinal) from message archive.

   \param mar A handle to the message archive.

   \param ord The zero-based ordinal position of the field.

   \param size An optional output parameter, in which, the size of the field
   value is set.

   \return A pointer to the field value or NULL on failure, in which case
   errno can be used to determine the error.

   \sa aug_valuebyname(), aug_getfield() and aug_nametoord().
*/

AUGMAR_API const void*
aug_valuebyord(aug_mar_t mar, unsigned ord, unsigned* size);

/**
   \brief Obtain field from message archive.

   \param mar A handle to the message archive.

   \param field The output parameter in which, the field will be returned.

   \param ord The zero-based ordinal position of the field.

   \return Zero on success, #AUG_RETNOMATCH if there is no matching field, or
   -1 on failure, in which case errno can be used to determine the error.

   \sa aug_valuebyname(), aug_valuebyord() and aug_getfields().
*/

AUGMAR_API int
aug_getfield(aug_mar_t mar, struct aug_field* field, unsigned ord);

/**
   \brief Obtain the number of fields contained within message archive.

   \param mar A handle to the message archive.

   \param size The output parameter, in which, the number of fields will be
   returned.

   \return Zero on success or -1 on failure, in which case errno can be used
   to determine the error.

   \sa aug_getfield().
*/

AUGMAR_API int
aug_getfields(aug_mar_t mar, unsigned* size);

/**
   \brief Obtain field name from ordinal position in message archive.

   \param mar A handle to the message archive.

   \param name The output parameter, in which, the name will be returned.

   \param ord The zero-based ordinal position of the name to be returned.

   \return Zero on success, #AUG_RETNOMATCH if there is no matching field, or
   -1 on failure, in which case errno can be used to determine the error.

   \sa aug_unsetbyname(), aug_valuebyname() and aug_nametoord().
*/

AUGMAR_API int
aug_ordtoname(aug_mar_t mar, const char** name, unsigned ord);

/**
   \brief Obtain ordinal position from field name in message archive.

   \param mar A handle to the message archive.

   \param ord The output parameter, in which, the ordinal position will be
   returned.

   \param name The name of the field.

   \return Zero on success, #AUG_RETNOMATCH if there is no matching field, or
   -1 on failure, in which case errno can be used to determine the error.

   \sa aug_unsetbyord(), aug_valuebyord() and aug_ordtoname().
*/

AUGMAR_API int
aug_nametoord(aug_mar_t mar, unsigned* ord, const char* name);

/**
   \brief Insert file into body content of message archive.

   \param mar A handle to the message archive.

   \param path A path to the file from which the content is read.

   \return Zero on success or -1 on failure, in which case errno can be used
   to determine the error.

   \sa aug_setcontent() and aug_writemar().
*/

AUGMAR_API int
aug_insertmar(aug_mar_t mar, const char* path);

/**
   \brief Reposition content offset within message archive.

   \param mar A handle to the message archive.

   \param offset The offset calculated according to the \ref SeekWhence
   "whence" directive.

   \param whence The \ref SeekWhence "whence" directive.

   \return The resulting offset location in bytes from the beginning of the
   content, or -1 on failure, in which case errno can be used to determine the
   error.

   \sa aug_readmar() and aug_writemar().
*/

AUGMAR_API off_t
aug_seekmar(aug_mar_t mar, off_t offset, int whence);

/**
   \brief Set content within message archive.

   \param mar A handle to the message archive.

   \param cdata A pointer to the content to be copied.

   \param size The size of the content to be copied.

   \return Zero on success or -1 on failure, in which case errno can be used
   to determine the error.

   \sa aug_insertmar() and aug_writemar().
*/

AUGMAR_API int
aug_setcontent(aug_mar_t mar, const void* cdata, unsigned size);

/**
   \brief Flush message archive buffers.

   \param mar A handle to the message archive.

   \return Zero on success or -1 on failure, in which case errno can be used
   to determine the error.
*/

AUGMAR_API int
aug_syncmar(aug_mar_t mar);

/**
   \brief Truncate content within message archive.

   \param mar A handle to the message archive.

   \param size The size to which the content will be truncated.

   \return Zero on success or -1 on failure, in which case errno can be used
   to determine the error.
*/

AUGMAR_API int
aug_truncatemar(aug_mar_t mar, unsigned size);

/**
   \brief Write content to message archive.

   \param mar A handle to the message archive.

   \param buf The buffer from which the content will be copied.

   \param len The number of bytes to be written.

   \return The number of bytes actually written or -1 on failure, in which
   case errno can be used to determine the error.

   \sa aug_insertmar(), aug_seekmar() and aug_setcontent().
*/

AUGMAR_API int
aug_writemar(aug_mar_t mar, const void* buf, unsigned len);

/**
   \brief Extract content from message archive into file.

   \param mar A handle to the message archive.

   \param path A path to the file into which the content will be written.

   \return Zero on success or -1 on failure, in which case errno can be used
   to determine the error.

   \sa aug_readmar() and aug_content().
*/

AUGMAR_API int
aug_extractmar(aug_mar_t mar, const char* path);

/**
   \brief Obtain content from message archive.

   \param mar A handle to the message archive.

   \param size An optional output parameter, in which, the size of the content
   is set.

   \return A pointer to the content or NULL on failure, in which case errno
   can be used to determine the error.

   \sa aug_readmar(), aug_extractmar() and aug_contentsize().
*/

AUGMAR_API const void*
aug_content(aug_mar_t mar, unsigned* size);

/**
   \brief Read content from message archive.

   \param mar A handle to the message archive.

   \param buf The output buffer into which the content will be copied.

   \param len The number of bytes to be read.

   \return The number of bytes actually read, or zero to signal end-of-file.
   -1 is returned on failure, in which case errno can be used to determine the
   error.

   \sa aug_seekmar(), aug_extractmar() and aug_content().
*/

AUGMAR_API int
aug_readmar(aug_mar_t mar, void* buf, unsigned len);

/**
   \brief Obtain size of content within message archive.

   \param mar A handle to the message archive.

   \param size The output parameter, in which, the size of the content will be
   returned.

   \return Zero on success or -1 on failure, in which case errno can be used
   to determine the error.

   \sa aug_content().
*/

AUGMAR_API int
aug_contentsize(aug_mar_t mar, unsigned* size);

#endif /* AUGMAR_MAR_H */
