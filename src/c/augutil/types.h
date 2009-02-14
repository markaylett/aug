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
#ifndef AUGUTIL_TYPES_H
#define AUGUTIL_TYPES_H

#define AUG_TOKERROR  (-1)
#define AUG_TOKLABEL  (-2)
#define AUG_TOKWORD   (-3)
#define AUG_TOKPHRASE (-4)
#define AUG_TOKRTRIM  (-5)

#define AUG_WRDESCAPE  0x01
#define AUG_WRDLABEL   0x02
#define AUG_WRDNEWLINE 0x04
#define AUG_WRDPAIRS   0x08

struct aug_words {
    void (*out_)(void*, int);
    void* arg_;
    void (*fn_)(struct aug_words*, int);
    unsigned flags_;
};

#endif /* AUGUTIL_TYPES_H */
