/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
AUGSYS_API int
aug_gettimeofday(struct timeval* tv, struct timezone* tz)
{
    return gettimeofday(tv, tz);
}
