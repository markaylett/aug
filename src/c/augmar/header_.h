/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
 * \file header.h
 * \brief TODO
 */

#ifndef AUGMAR_HEADER_H_
#define AUGMAR_HEADER_H_

#include "augmar/seq_.h"

struct aug_info_;

AUGMAR_EXTERN int
aug_removefields_(aug_seq_t seq, struct aug_info_* info);

AUGMAR_EXTERN int
aug_setfield_(aug_seq_t seq, struct aug_info_* info,
              const struct aug_field* field, size_t* ord);

AUGMAR_EXTERN int
aug_setvalue_(aug_seq_t seq, struct aug_info_* info, size_t ord,
              const void* value, size_t size);

AUGMAR_EXTERN int
aug_unsetbyname_(aug_seq_t seq, struct aug_info_* info, const char* name,
                 size_t* ord);

AUGMAR_EXTERN int
aug_unsetbyord_(aug_seq_t seq, struct aug_info_* info, size_t ord);

AUGMAR_EXTERN const void*
aug_valuebyname_(aug_seq_t seq, const struct aug_info_* info,
                 const char* name, size_t* size);

AUGMAR_EXTERN const void*
aug_valuebyord_(aug_seq_t seq, const struct aug_info_* info, size_t ord,
                size_t* size);

AUGMAR_EXTERN int
aug_field_(aug_seq_t seq, const struct aug_info_* info,
           struct aug_field* field, size_t ord);

AUGMAR_EXTERN int
aug_ordtoname_(aug_seq_t seq, const struct aug_info_* info, const char** name,
               size_t ord);

AUGMAR_EXTERN int
aug_nametoord_(aug_seq_t seq, const struct aug_info_* info, size_t* ord,
               const char* name);

#endif /* AUGMAR_HEADER_H_ */
