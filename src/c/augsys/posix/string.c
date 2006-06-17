/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
AUGSYS_API const char*
aug_strerror(int errnum)
{
    return strerror(errnum);
}
