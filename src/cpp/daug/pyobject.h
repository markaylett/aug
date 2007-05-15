/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef PYOBJECT_H
#define PYOBJECT_H

#if defined(_WIN32)
# undef _DEBUG
#endif /* _WIN32 */

#include <Python.h>

struct augas_host;

PyTypeObject*
pycreatetype(const struct augas_host* host);

PyObject*
pycreateobject(PyTypeObject* type, const char* sname, int id, PyObject* user);

void
pysetid(PyObject* self, int id);

int
pygetid(PyObject* self);

void
pysetuser(PyObject* self, PyObject* user);

PyObject*
pygetuser(PyObject* self);

void
pycheckobjects(void);

#endif /* PYOBJECT_H */
