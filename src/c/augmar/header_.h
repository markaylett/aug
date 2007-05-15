/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
   \file header.h
   TODO
 */

#ifndef AUGMAR_HEADER_H_
#define AUGMAR_HEADER_H_

#include "augmar/seq_.h"

struct aug_info_;

AUG_EXTERN int
aug_removefields_(aug_seq_t seq, struct aug_info_* info);

AUG_EXTERN int
aug_setfield_(aug_seq_t seq, struct aug_info_* info,
              const struct aug_field* field, unsigned* ord);

AUG_EXTERN int
aug_setvalue_(aug_seq_t seq, struct aug_info_* info, unsigned ord,
              const void* value, unsigned size);

AUG_EXTERN int
aug_unsetbyname_(aug_seq_t seq, struct aug_info_* info, const char* name,
                 unsigned* ord);

AUG_EXTERN int
aug_unsetbyord_(aug_seq_t seq, struct aug_info_* info, unsigned ord);

AUG_EXTERN const void*
aug_valuebyname_(aug_seq_t seq, const struct aug_info_* info,
                 const char* name, unsigned* size);

AUG_EXTERN const void*
aug_valuebyord_(aug_seq_t seq, const struct aug_info_* info, unsigned ord,
                unsigned* size);

AUG_EXTERN int
aug_getfield_(aug_seq_t seq, const struct aug_info_* info,
              struct aug_field* field, unsigned ord);

AUG_EXTERN int
aug_ordtoname_(aug_seq_t seq, const struct aug_info_* info, const char** name,
               unsigned ord);

AUG_EXTERN int
aug_nametoord_(aug_seq_t seq, const struct aug_info_* info, unsigned* ord,
               const char* name);

#endif /* AUGMAR_HEADER_H_ */
