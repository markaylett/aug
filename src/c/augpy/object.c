/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGPY_BUILD
#include "augpy/object.h"

#include <structmember.h>

#include <augas.h>

/* Implementation note: always reassign members before decrementing reference
   counts. */

static const struct augas_host* host_ = NULL;
static int objects_ = 0;

typedef struct {
    PyObject_HEAD
    PyObject* sname_;
    int id_;
    PyObject* user_;
} object_;

static PyMemberDef members_[] = {
    {
        "user", T_OBJECT_EX, offsetof(object_, user_), 0, "TODO"
    },
    { NULL }
};

static int
clear_(object_* self)
{
    PyObject* tmp;

    tmp = self->sname_;
    self->sname_ = NULL;
    Py_DECREF(tmp);

    tmp = self->user_;
    self->user_ = NULL;
    Py_DECREF(tmp);

    return 0;
}

static void
dealloc_(object_* self)
{
    --objects_;
    host_->writelog_(AUGAS_LOGDEBUG,
                     "deallocated: <augas.Object at %p, sname='%s', id=%d>",
                     (void*)self, PyString_AsString(self->sname_), self->id_);

    clear_(self);
    self->ob_type->tp_free((PyObject*)self);
}

static int
compare_(object_* lhs, object_* rhs)
{
    int ret;
    if (lhs->id_ < rhs->id_)
        ret = -1;
    else if (lhs->id_ > rhs->id_)
        ret = 1;
    else
        ret = 0;
    return ret;
}

static PyObject*
repr_(object_* self)
{
    return PyString_FromFormat("<augas.Object at %p, sname='%s', id=%d>",
                               (void*)self, PyString_AsString(self->sname_),
                               self->id_);
}

static long
hash_(object_* self)
{
    /* Must not return -1. */

    return -1 == self->id_ ? 0 : self->id_;
}

static PyObject*
str_(object_* self)
{
    return PyString_FromFormat("('%s', %d)", PyString_AsString(self->sname_),
                               self->id_);
}

static int
traverse_(object_* self, visitproc visit, void* arg)
{
    int ret;

    if (self->sname_) {
        ret = visit(self->sname_, arg);
        if (ret != 0)
            return ret;
    }

    if (self->user_) {
        ret = visit(self->user_, arg);
        if (ret != 0)
            return ret;
    }

    return 0;
}

static int
init_(object_* self, PyObject* args, PyObject* kwds)
{
    PyObject* sname = NULL, * user = NULL, * tmp;

    static char* kwlist[] = { "sname", "id", "user", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Si|O", kwlist,
                                     &sname, &self->id_, &user))
        return -1;

    if (sname) {
        tmp = self->sname_;
        Py_INCREF(sname);
        self->sname_ = sname;
        Py_DECREF(tmp);
    }

    if (user) {
        tmp = self->user_;
        Py_INCREF(user);
        self->user_ = user;
        Py_DECREF(tmp);
    }

    return 0;
}

static PyObject*
new_(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    object_* self;

    self = (object_*)type->tp_alloc(type, 0);
    if (self) {

        if (!(self->sname_ = PyString_FromString(""))) {
            Py_DECREF(self);
            return NULL;
        }

        self->id_ = 0;

        Py_INCREF(Py_None);
        self->user_ = Py_None;
    }

    ++objects_;
    host_->writelog_(AUGAS_LOGDEBUG,
                     "allocated: <augas.Object at %p, sname='%s', id=%d>",
                     (void*)self, PyString_AsString(self->sname_), self->id_);
    return (PyObject*)self;
}

static PyObject*
getsname_(object_* self, void *closure)
{
    Py_INCREF(self->sname_);
    return self->sname_;
}

static PyObject*
getid_(object_* self, void *closure)
{
    return Py_BuildValue("i", self->id_);
}

static PyGetSetDef getset_[] = {
    {
        "sname", (getter)getsname_, NULL, "TODO", NULL
    },
    {
        "id", (getter)getid_, NULL, "TODO", NULL
    },
    { NULL }  /* Sentinel */
};

static PyTypeObject pytype_ = {
    PyObject_HEAD_INIT(NULL)
    0,                       /*ob_size*/
    "augas.Object",          /*tp_name*/
    sizeof(object_),         /*tp_basicsize*/
    0,                       /*tp_itemsize*/
    (destructor)dealloc_,    /*tp_dealloc*/
    0,                       /*tp_print*/
    0,                       /*tp_getattr*/
    0,                       /*tp_setattr*/
    (cmpfunc)compare_,       /*tp_compare*/
    (reprfunc)repr_,         /*tp_repr*/
    0,                       /*tp_as_number*/
    0,                       /*tp_as_sequence*/
    0,                       /*tp_as_mapping*/
    (hashfunc)hash_,         /*tp_hash */
    0,                       /*tp_call*/
    (reprfunc)str_,          /*tp_str*/
    0,                       /*tp_getattro*/
    0,                       /*tp_setattro*/
    0,                       /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE
    | Py_TPFLAGS_HAVE_GC,    /*tp_flags*/
    "TODO",                  /* tp_doc */
    (traverseproc)traverse_, /* tp_traverse */
    (inquiry)clear_,         /* tp_clear */
    0,                       /* tp_richcompare */
    0,                       /* tp_weaklistoffset */
    0,                       /* tp_iter */
    0,                       /* tp_iternext */
    0,                       /* tp_methods */
    members_,                /* tp_members */
    getset_,                 /* tp_getset */
    0,                       /* tp_base */
    0,                       /* tp_dict */
    0,                       /* tp_descr_get */
    0,                       /* tp_descr_set */
    0,                       /* tp_dictoffset */
    (initproc)init_,         /* tp_init */
    0,                       /* tp_alloc */
    new_,                    /* tp_new */
};

PyTypeObject*
augpy_createtype(const struct augas_host* host)
{
    host_ = host;
    if (-1 == PyType_Ready(&pytype_))
        return NULL;

    Py_INCREF(&pytype_);
    return &pytype_;
}

PyObject*
augpy_createobject(PyTypeObject* type, const char* sname, int id,
                   PyObject* user)
{
    object_* self = PyObject_GC_New(object_, type);
    if (!self)
        return NULL;

    if (!(self->sname_ = PyString_FromString(sname))) {
        Py_DECREF(self);
        return NULL;
    }

    self->id_ = id;

    if (!user)
        user = Py_None;
    Py_INCREF(user);
    self->user_ = user;

    ++objects_;
    host_->writelog_(AUGAS_LOGDEBUG,
                     "allocated: <augas.Object at %p, sname='%s', id=%d>",
                     (void*)self, PyString_AsString(self->sname_), self->id_);
    return (PyObject*)self;
}

void
augpy_setid(PyObject* self, int id)
{
    object_* x = (object_*)self;
    x->id_ = id;
}

int
augpy_getid(PyObject* self)
{
    object_* x = (object_*)self;
    return x->id_;
}

void
augpy_setuser(PyObject* self, PyObject* user)
{
    object_* x = (object_*)self;
    PyObject* tmp = x->user_;
    Py_INCREF(user);
    x->user_ = user;
    Py_DECREF(tmp);
}

PyObject*
augpy_getuser(PyObject* self)
{
    object_* x = (object_*)self;
    Py_INCREF(x->user_);
    return x->user_;
}

int
augpy_objects(void)
{
    return objects_;
}
