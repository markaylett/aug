#if defined(_WIN32)
# undef _DEBUG
#endif /* _WIN32 */

#include <Python.h>
#include <structmember.h>

#include <augas.h>

static const struct augas_host* host_ = NULL;
static int objects_ = 0;

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

static int
pyclear_(pyobject* self)
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
pydealloc_(pyobject* self)
{
    --objects_;
    host_->writelog_(AUGAS_LOGDEBUG,
                     "deallocated: <augas.Object at %p, sname='%s', id=%d>",
                     (void*)self, PyString_AsString(self->sname_), self->id_);

    pyclear_(self);
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

static PyObject*
pyrepr_(pyobject* self)
{
    return PyString_FromFormat("<augas.Object at %p, sname='%s', id=%d>",
                               (void*)self, PyString_AsString(self->sname_),
                               self->id_);
}

static long
pyhash_(pyobject* self)
{
    /* Must not return -1. */

    return -1 == self->id_ ? 0 : self->id_;
}

static PyObject*
pystr_(pyobject* self)
{
    return PyString_FromFormat("('%s', %d)", PyString_AsString(self->sname_),
                               self->id_);
}

static int
pytraverse_(pyobject* self, visitproc visit, void* arg)
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
pyinit_(pyobject* self, PyObject* args, PyObject* kwds)
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

    ++objects_;
    host_->writelog_(AUGAS_LOGDEBUG,
                     "allocated: <augas.Object at %p, sname='%s', id=%d>",
                     (void*)self, PyString_AsString(self->sname_), self->id_);
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
    (reprfunc)pyrepr_,      /*tp_repr*/
    0,                      /*tp_as_number*/
    0,                      /*tp_as_sequence*/
    0,                      /*tp_as_mapping*/
    (hashfunc)pyhash_,      /*tp_hash */
    0,                      /*tp_call*/
    (reprfunc)pystr_,       /*tp_str*/
    0,                      /*tp_getattro*/
    0,                      /*tp_setattro*/
    0,                      /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, /*tp_flags*/
    "TODO",                 /* tp_doc */
    (traverseproc)pytraverse_, /* tp_traverse */
    (inquiry)pyclear_,      /* tp_clear */
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
pycreatetype(const struct augas_host* host)
{
    host_ = host;
    if (-1 == PyType_Ready(&pytype_))
        return NULL;

    Py_INCREF(&pytype_);
    return &pytype_;
}

PyObject*
pycreateobject(PyTypeObject* type, const char* sname, int id, PyObject* user)
{
    pyobject* self = PyObject_GC_New(pyobject, type);
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
pysetid(PyObject* self, int id)
{
    pyobject* x = (pyobject*)self;
    x->id_ = id;
}

int
pygetid(PyObject* self)
{
    pyobject* x = (pyobject*)self;
    return x->id_;
}

void
pysetuser(PyObject* self, PyObject* user)
{
    pyobject* x = (pyobject*)self;
    PyObject* tmp = x->user_;
    Py_INCREF(user);
    x->user_ = user;
    Py_DECREF(tmp);
}

PyObject*
pygetuser(PyObject* self)
{
    pyobject* x = (pyobject*)self;
    Py_INCREF(x->user_);
    return x->user_;
}

void
pycheckobjects(void)
{
    int level = objects_ ? AUGAS_LOGERROR : AUGAS_LOGINFO;
    host_->writelog_(level, "allocated objects: %d", objects_);
}
