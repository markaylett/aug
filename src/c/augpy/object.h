/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGPY_OBJECT_H
#define AUGPY_OBJECT_H

#if defined(_WIN32)
# undef _DEBUG
#endif /* _WIN32 */

#include <Python.h>

struct augas_host;

PyTypeObject*
augpy_createtype(const struct augas_host* host);

PyObject*
augpy_createobject(PyTypeObject* type, const char* sname, int id,
                   PyObject* user);

void
augpy_setid(PyObject* self, int id);

int
augpy_getid(PyObject* self);

void
augpy_setuser(PyObject* self, PyObject* user);

PyObject*
augpy_getuser(PyObject* self);

void
augpy_checkobjects(void);

#endif /* AUGPY_OBJECT_H */
