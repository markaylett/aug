/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
   \file utility.h
   TODO
 */

#ifndef MAR_UTILITY_H
#define MAR_UTILITY_H

#include "mar/config.h"

#include "augmar/mar.h"

#include <stdio.h>

AUG_EXTERNC int
aug_atofield_(struct aug_field* field, char* src);

AUG_EXTERNC int
aug_confirm_(const char* prompt);

AUG_EXTERNC int
aug_insertstream_(aug_mar_t mar, FILE* stream);

AUG_EXTERNC ssize_t
aug_readline_(char* buf, size_t size, FILE* stream);

AUG_EXTERNC int
aug_streamset_(aug_mar_t mar, FILE* stream);

AUG_EXTERNC int
aug_writevalue_(FILE* stream, const void* value, size_t size);

#endif /* MAR_UTILITY_H */
