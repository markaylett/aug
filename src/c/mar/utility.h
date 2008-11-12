/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef MAR_UTILITY_H
#define MAR_UTILITY_H

#include "mar/config.h"

#include "augmar/mar.h"

#include <stdio.h>

AUG_EXTERNC aug_result
aug_atofield_(struct aug_field* field, char* src);

AUG_EXTERNC aug_bool
aug_confirm_(const char* prompt);

AUG_EXTERNC aug_result
aug_insertstream_(aug_mar_t mar, FILE* stream);

AUG_EXTERNC aug_rsize
aug_readline_(char* buf, size_t size, FILE* stream);

AUG_EXTERNC aug_result
aug_streamset_(aug_mar_t mar, FILE* stream);

AUG_EXTERNC aug_result
aug_writevalue_(FILE* stream, const void* value, size_t size);

#endif /* MAR_UTILITY_H */
