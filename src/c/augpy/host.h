/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGPY_HOST_H
#define AUGPY_HOST_H

#if defined(_WIN32)
# undef _DEBUG
#endif /* _WIN32 */

#include <Python.h>

PyObject*
augpy_createhost(PyTypeObject* type);

#endif /* AUGPY_HOST_H */
