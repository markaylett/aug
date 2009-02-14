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

#include "augctx/utility.h" /* aug_check() */

#include <stdio.h>
#include <stdlib.h>         /* abort() */

AUG_EXTERNC aug_bool
aug_createtlskey_(aug_tlskey_t* tlskey)
{
    return 0xffffffff != (*tlskey = TlsAlloc())
        ? AUG_TRUE : AUG_FALSE;
}

AUG_EXTERNC void
aug_destroytlskey_(aug_tlskey_t tlskey)
{
    aug_check(TlsFree(tlskey));
}

AUG_EXTERNC void
aug_settlsvalue_(aug_tlskey_t tlskey, void* value)
{
    aug_check(TlsSetValue(tlskey, value));
}

AUG_EXTERNC void*
aug_gettlsvalue_(aug_tlskey_t tlskey)
{
    void* ret = TlsGetValue(tlskey);
    aug_check(ret || NO_ERROR == GetLastError());
    return ret;
}
