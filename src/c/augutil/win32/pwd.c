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
#include <conio.h>
#include <stdio.h>

AUGUTIL_API char*
aug_getpass(const char* prompt, char* buf, size_t len)
{
    char ch;
    if (!len)
        return NULL;
    _cputs(prompt);
    while (--len)
        switch (ch = _getch()) {
        case 0x03: /* CTRL+C */
            *buf = '\0';
            _cputs("\n");
            return NULL;
        case 0x04: /* CTRL+D */
        case '\r':
        case '\n':
        case EOF:
            goto done;
        default:

            /* Re-read for the actual key value if necessary. */

            if (!ch || ch == (char)0xE0)
                ch = _getch();
            *buf++ = ch;
        }
 done:
    *buf = '\0';
    _cputs("\n");
    return buf;
}
