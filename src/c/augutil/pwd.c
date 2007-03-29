/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/pwd.h"

static const char rcsid[] = "$Id:$";

#if !defined(_WIN32)
# include "augutil/posix/pwd.c"
#else /* _WIN32 */
# include "augutil/win32/pwd.c"
#endif /* _WIN32 */
