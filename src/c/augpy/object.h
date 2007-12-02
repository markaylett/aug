/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGPY_OBJECT_H
#define AUGPY_OBJECT_H

#if defined(_WIN32)
# undef _DEBUG
#endif /* _WIN32 */

#include "maud.h"
#include "augobj/blob.h"

#include <Python.h>

AUG_OBJECTDECL(augpy_blob);

struct augpy_blobvtbl {
    AUG_OBJECT(augpy_blob);
    PyObject* (*get_)(augpy_blob*);
};

aug_blob*
augpy_createblob(PyObject* pyobj);

const void*
augpy_blobdata(aug_object* obj, size_t* size);

PyTypeObject*
augpy_createtype(void);

PyObject*
augpy_createobject(PyTypeObject* type, int id, PyObject* user);

void
augpy_setid(PyObject* self, int id);

int
augpy_getid(PyObject* self);

void
augpy_setuser(PyObject* self, PyObject* user);

PyObject*
augpy_getuser(PyObject* self);

int
augpy_objects(void);

#endif /* AUGPY_OBJECT_H */
