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
#ifndef AUGRB_OBJECT_H
#define AUGRB_OBJECT_H

#include "augmod.h"
#include "augses.h"
#include "augext/blob.h"

#if !defined(_WIN32)
# include <unistd.h>
#else /* _WIN32 */
# define HAVE_ISINF 1
# if !defined(_MSC_VER)
#  define _MSC_VER 1200
# else /* _MSC_VER */
#  undef _MSC_VER
#  define _MSC_VER 1200
#  pragma comment(lib, "msvcrt-ruby18.lib")
# endif /* _MSC_VER */
char*
rb_w32_getcwd(char* buffer, int size);
#endif /* _WIN32 */

#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include <ruby.h>

AUG_INTERFACE(augrb_box);

struct augrb_boxvtbl {
    AUG_VTBL(augrb_box);
    VALUE (*unbox_)(augrb_box*);
};

augrb_box*
augrb_createbox(VALUE rbob);

aug_blob*
augrb_createblob(VALUE rbob);

VALUE
augrb_obtorb(aug_object* ob);

#endif /* AUGRB_OBJECT_H */
