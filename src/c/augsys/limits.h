/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_LIMITS_H
#define AUGSYS_LIMITS_H

#if !defined(_WIN32)
# include <limits.h>
# if !defined(PATH_MAX)
#  define AUG_PATH_MAX 255
# else /* PATH_MAX */
#  define AUG_PATH_MAX PATH_MAX
# endif /* PATH_MAX */
#else /* _WIN32 */
# include <stdlib.h>
# if !defined(_MAX_PATH)
#  define AUG_PATH_MAX 260
# else /* _MAX_PATH */
#  define AUG_PATH_MAX _MAX_PATH
# endif /* _MAX_PATH */
#endif /* _WIN32 */

#endif /* AUGSYS_LIMITS_H */
