#if defined(_WIN32)
# undef _DEBUG
#endif /* _WIN32 */

#include <Python.h>
#include <structmember.h>

typedef struct {
    PyObject_HEAD
    PyObject* sname_;
    int id_;
    PyObject* user_;
} pyobject;

static PyMemberDef pymembers_[] = {
    {
        "user", T_OBJECT_EX, offsetof(pyobject, user_), 0, "TODO"
    },
    { NULL }
};

static void
pydealloc_(pyobject* self)
{
    Py_XDECREF(self->sname_);
    Py_XDECREF(self->user_);
    self->ob_type->tp_free((PyObject*)self);
}

static int
pycompare_(pyobject* lhs, pyobject* rhs)
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

static int
pyinit_(pyobject* self, PyObject* args, PyObject* kwds)
{
    PyObject* sname = NULL, * user = NULL;

    static char* kwlist[] = { "sname", "id", "user", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Si|O", kwlist,
                                     &sname, &self->id_, &user))
        return -1;

    if (sname) {
        Py_DECREF(self->sname_);
        Py_INCREF(sname);
        self->sname_ = sname;
    }

    if (user) {
        Py_DECREF(self->user_);
        Py_INCREF(user);
        self->user_ = user;
    }

    return 0;
}

static PyObject*
pynew_(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    pyobject* self;

    self = (pyobject*)type->tp_alloc(type, 0);
    if (self) {

        if (!(self->sname_ = PyString_FromString(""))) {
            Py_DECREF(self);
            return NULL;
        }

        self->id_ = 0;

        Py_INCREF(Py_None);
        self->user_ = Py_None;
    }

    return (PyObject*)self;
}

static PyObject*
pygetsname_(pyobject* self, void *closure)
{
    Py_INCREF(self->sname_);
    return self->sname_;
}

static PyObject*
pygetid_(pyobject* self, void *closure)
{
    return Py_BuildValue("i", self->id_);
}

static PyGetSetDef pygetset_[] = {
    {
        "sname", (getter)pygetsname_, NULL, "TODO", NULL
    },
    {
        "id", (getter)pygetid_, NULL, "TODO", NULL
    },
    { NULL }  /* Sentinel */
};

static PyTypeObject pytype_ = {
    PyObject_HEAD_INIT(NULL)
    0,                      /*ob_size*/
    "augas.Object",         /*tp_name*/
    sizeof(pyobject),       /*tp_basicsize*/
    0,                      /*tp_itemsize*/
    (destructor)pydealloc_, /*tp_dealloc*/
    0,                      /*tp_print*/
    0,                      /*tp_getattr*/
    0,                      /*tp_setattr*/
    (cmpfunc)pycompare_,    /*tp_compare*/
    0,                      /*tp_repr*/
    0,                      /*tp_as_number*/
    0,                      /*tp_as_sequence*/
    0,                      /*tp_as_mapping*/
    0,                      /*tp_hash */
    0,                      /*tp_call*/
    0,                      /*tp_str*/
    0,                      /*tp_getattro*/
    0,                      /*tp_setattro*/
    0,                      /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "TODO",                 /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    0,                      /* tp_methods */
    pymembers_,             /* tp_members */
    pygetset_,              /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    (initproc)pyinit_,      /* tp_init */
    0,                      /* tp_alloc */
    pynew_,                 /* tp_new */
};

PyTypeObject*
pycreatetype(void)
{
    if (-1 == PyType_Ready(&pytype_))
        return NULL;

    Py_INCREF(&pytype_);
    return &pytype_;
}

PyObject*
pycreateobject(PyTypeObject* type, const char* sname, int id, PyObject* user)
{
    pyobject* self = PyObject_New(pyobject, type);
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

    return (PyObject*)self;
}

void
pysetid(PyObject* self, int id)
{
    pyobject* x = (pyobject*)self;
    x->id_ = id;
}

void
pysetuser(PyObject* self, PyObject* user)
{
    pyobject* x = (pyobject*)self;
    Py_DECREF(x->user_);
    Py_INCREF(user);
    x->user_ = user;
}

PyObject*
pygetuser(PyObject* self)
{
    pyobject* x = (pyobject*)self;
    Py_INCREF(x->user_);
    return x->user_;
}
