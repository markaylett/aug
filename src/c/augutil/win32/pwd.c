/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
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
