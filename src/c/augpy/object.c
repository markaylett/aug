/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define MAUD_BUILD
#include "augpy/object.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include <structmember.h>

struct blobimpl_ {
    aug_blob blob_;
    augpy_blob pyblob_;
    unsigned refs_;
    PyObject* pyobj_;
};

static void*
castblob_(aug_blob* obj, const char* id)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, obj);
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_blobid)) {
        aug_incref(&impl->blob_);
        return &impl->blob_;
    } else if (AUG_EQUALID(id, augpy_blobid)) {
        aug_incref(&impl->pyblob_);
        return &impl->pyblob_;
    }
    return NULL;
}

static int
increfblob_(aug_blob* obj)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, obj);
    ++impl->refs_;
    return 0;
}

static int
decrefblob_(aug_blob* obj)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, obj);
    if (0 == --impl->refs_) {
        Py_DECREF(impl->pyobj_);
        free(impl);
    }
    return 0;
}

static const void*
blobdata_(aug_blob* obj, size_t* size)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, obj);
    const void* data;
    int len;

    if (-1 == PyObject_AsReadBuffer(impl->pyobj_, &data, &len)) {
        if (size)
            *size = 0;
        return NULL;
    }

    if (size)
        *size = len;
    return data;
}

static const struct aug_blobvtbl blobvtbl_ = {
    castblob_,
    increfblob_,
    decrefblob_,
    blobdata_
};

static void*
castpyblob_(augpy_blob* obj, const char* id)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, pyblob_, obj);
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, augpy_blobid)) {
        aug_incref(&impl->pyblob_);
        return &impl->pyblob_;
    } else if (AUG_EQUALID(id, aug_blobid)) {
        aug_incref(&impl->blob_);
        return &impl->blob_;
    }
    return NULL;
}

static int
increfpyblob_(augpy_blob* obj)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, pyblob_, obj);
    ++impl->refs_;
    return 0;
}

static int
decrefpyblob_(augpy_blob* obj)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, pyblob_, obj);
    if (0 == --impl->refs_) {
        Py_DECREF(impl->pyobj_);
        free(impl);
    }
    return 0;
}

static PyObject*
getpyblob_(augpy_blob* obj)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, pyblob_, obj);
    Py_INCREF(impl->pyobj_);
    return impl->pyobj_;
}

static const struct augpy_blobvtbl pyblobvtbl_ = {
    castpyblob_,
    increfpyblob_,
    decrefpyblob_,
    getpyblob_
};

aug_blob*
augpy_createblob(PyObject* pyobj)
{
    struct blobimpl_* impl = malloc(sizeof(struct blobimpl_));
    if (!impl)
        return NULL;

    if (!pyobj)
        pyobj = Py_None;

    impl->blob_.vtbl_ = &blobvtbl_;
    impl->blob_.impl_ = NULL;

    impl->pyblob_.vtbl_ = &pyblobvtbl_;
    impl->pyblob_.impl_ = NULL;

    impl->refs_ = 1;
    impl->pyobj_ = pyobj;

    Py_INCREF(impl->pyobj_);
    return &impl->blob_;
}

const void*
augpy_blobdata(aug_object* obj, size_t* size)
{
    const void* data = NULL;
    if (obj) {
        aug_blob* blob = aug_cast(obj, aug_blobid);
        if (blob) {
            data = aug_blobdata(blob, size);
            aug_decref(blob);
        }
    }
    return data;
}

PyObject*
augpy_getblob(aug_object* obj)
{
    PyObject* pyobj = NULL;
    if (obj) {
        augpy_blob* blob = aug_cast(obj, augpy_blobid);
        if (blob) {
            pyobj = blob->vtbl_->get_(blob);
            aug_decref(blob);
        }
    }
    return pyobj;
}

/* Implementation note: always reassign members before decrementing reference
   counts. */

static int objects_ = 0;

typedef struct {
    PyObject_HEAD
    char name_[MAUD_MAXNAME + 1];
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

    tmp = self->user_;
    self->user_ = NULL;
    Py_DECREF(tmp);

    return 0;
}

static void
dealloc_(object_* self)
{
    --objects_;
    maud_writelog(MAUD_LOGDEBUG, "deallocated: <augpy.Object at %p, id=%d>",
                  (void*)self, self->id_);

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
    return PyString_FromFormat("<augpy.Object at %p, id=%d>",
                               (void*)self, self->id_);
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
    return PyString_FromFormat("%d", self->id_);
}

static int
traverse_(object_* self, visitproc visit, void* arg)
{
    int ret;

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
    PyObject* user = NULL, * tmp;

    static char* kwlist[] = { "id", "user", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i|O", kwlist, &self->id_,
                                     &user))
        return -1;

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

        self->id_ = 0;

        Py_INCREF(Py_None);
        self->user_ = Py_None;
    }

    ++objects_;
    maud_writelog(MAUD_LOGDEBUG, "allocated: <augpy.Object at %p, id=%d>",
                  (void*)self, self->id_);
    return (PyObject*)self;
}

static PyObject*
getid_(object_* self, void *closure)
{
    return Py_BuildValue("i", self->id_);
}

static PyGetSetDef getset_[] = {
    {
        "id", (getter)getid_, NULL, "TODO", NULL
    },
    { NULL }  /* Sentinel */
};

static PyTypeObject pytype_ = {
    PyObject_HEAD_INIT(NULL)
    0,                       /*ob_size*/
    "augpy.Object",          /*tp_name*/
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
augpy_createtype(void)
{
    if (-1 == PyType_Ready(&pytype_))
        return NULL;

    Py_INCREF(&pytype_);
    return &pytype_;
}

PyObject*
augpy_createobject(PyTypeObject* type, int id, PyObject* user)
{
    object_* self = PyObject_GC_New(object_, type);
    if (!self)
        return NULL;

    self->id_ = id;

    if (!user)
        user = Py_None;
    Py_INCREF(user);
    self->user_ = user;

    ++objects_;
    maud_writelog(MAUD_LOGDEBUG, "allocated: <augpy.Object at %p, id=%d>",
                  (void*)self, self->id_);
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
