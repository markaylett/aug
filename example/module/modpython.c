#if defined(_WIN32)
# undef _DEBUG
#endif /* _WIN32 */

#include <Python.h>
#include <augas.h>

#include <stdlib.h>

struct import_ {
    PyObject* module_;
    PyObject* closesess_;
    PyObject* opensess_;
    PyObject* event_;
    PyObject* expire_;
    PyObject* reconf_;
    PyObject* closeconn_;
    PyObject* openconn_;
    PyObject* notconn_;
    PyObject* data_;
    PyObject* rdexpire_;
    PyObject* wrexpire_;
    PyObject* teardown_;
    int open_;
};

struct link_ {
    PyObject* object_;
    struct link_* next_;
};

static const struct augas_host* host_ = NULL;
static PyObject* pyaugas_ = NULL;

static struct link_* list_ = NULL;
static int opencall_ = 0;

static void
retain_(void)
{
    while (list_) {
        struct link_* p = list_;
        list_ = p->next_;
        free(p);
    }
}

static void
release_(void)
{
    while (list_) {
        struct link_* p = list_;
        list_ = p->next_;
        Py_CLEAR(p->object_);
        free(p);
    }
}

static void
pushuser_(PyObject* x)
{
    struct link_* p = malloc(sizeof(struct link_));
    if (p) {
        p->object_ = x;
        p->next_ = list_;
        list_ = p;
    }
}

static void
printerr_(void)
{
    PyObject* type, * value, * traceback;
    PyObject* module;

    PyErr_Fetch(&type, &value, &traceback);
    if ((module = PyImport_ImportModule("traceback"))) {

        PyObject* list, * empty, * message;
        list = PyObject_CallMethod(module, "format_exception", "OOO", type,
                                   value == NULL ? Py_None : value,
                                   traceback == NULL ? Py_None : traceback);

        empty = PyString_FromString("");
        message = PyObject_CallMethod(empty, "join", "O", list);
        host_->writelog_(AUGAS_LOGERROR, "%s", PyString_AsString(message));
        Py_DECREF(message);
        Py_DECREF(empty);
        Py_DECREF(list);
        Py_DECREF(module);
    } else
        PyErr_Print();

    Py_DECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);
}

static int
check_(PyObject* x)
{
    if (!x) {
        if (PyErr_Occurred()) {
            printerr_();
            PyErr_Clear();
        }
        return -1;
    }
    return 0;
}

static PyObject*
getmodule_(const char* name)
{
    PyObject* s, * m;
    s = PyString_FromString(name);
    m = PyImport_Import(s);
	check_(m);
    Py_DECREF(s);
    return m;
}

static PyObject*
getmethod_(PyObject* module, const char* name)
{
    PyObject* x = PyObject_GetAttrString(module, name);
    if (x) {
        if (!PyCallable_Check(x)) {
            Py_CLEAR(x);
        }
    } else
        PyErr_Clear();
    return x;
}

static void
freeimport_(struct import_* import)
{
    if (import->open_ && import->closesess_) {

        const char* sname = PyModule_GetName(import->module_);
        if (sname) {
            PyObject* x = PyObject_CallFunction(import->closesess_, "s",
                                                sname);
            if (0 == check_(x)) {
                Py_DECREF(x);
            }
        }
    }

    Py_CLEAR(import->teardown_);
    Py_CLEAR(import->wrexpire_);
    Py_CLEAR(import->rdexpire_);
    Py_CLEAR(import->data_);
    Py_CLEAR(import->notconn_);
    Py_CLEAR(import->openconn_);
    Py_CLEAR(import->closeconn_);
    Py_CLEAR(import->reconf_);
    Py_CLEAR(import->expire_);
    Py_CLEAR(import->event_);
    Py_CLEAR(import->opensess_);
    Py_CLEAR(import->closesess_);
    Py_CLEAR(import->module_);
    free(import);
}

static struct import_*
createimport_(const char* sname)
{
    struct import_* import = malloc(sizeof(struct import_));
    if (!import)
        return NULL;

    import->open_ = 0;
    if (!(import->module_ = getmodule_(sname)))
        goto fail;

    import->closesess_ = getmethod_(import->module_, "closesess");
    import->opensess_ = getmethod_(import->module_, "opensess");
    import->event_ = getmethod_(import->module_, "event");
    import->expire_ = getmethod_(import->module_, "expire");
    import->reconf_ = getmethod_(import->module_, "reconf");
    import->closeconn_ = getmethod_(import->module_, "closeconn");
    import->openconn_ = getmethod_(import->module_, "openconn");
    import->notconn_ = getmethod_(import->module_, "notconn");
    import->data_ = getmethod_(import->module_, "data");
    import->rdexpire_ = getmethod_(import->module_, "rdexpire");
    import->wrexpire_ = getmethod_(import->module_, "wrexpire");
    import->teardown_ = getmethod_(import->module_, "teardown");

    if (import->opensess_) {
        PyObject* x;
        opencall_ = 1;
        x = PyObject_CallFunction(import->opensess_, "s", sname);
        opencall_ = 0;
        if (-1 == check_(x)) {
            release_();
            goto fail;
        }
        retain_();
        Py_DECREF(x);
    }

    import->open_ = 1;
    return import;

 fail:
    freeimport_(import);
    return NULL;
}

static PyObject*
pygetenv_(PyObject* self, PyObject* args)
{
    const char* sname, * name, * value;

    if (!(sname = PyModule_GetName(self)))
        return NULL;

    if (!PyArg_ParseTuple(args, "ss:getenv", &sname, &name))
        return NULL;

    if (!(value = host_->getenv_(sname, name)))
        Py_RETURN_NONE;

    return Py_BuildValue("s", value);
}

static PyObject*
pywritelog_(PyObject* self, PyObject* args)
{
    int level;
    const char* msg;

    if (!PyArg_ParseTuple(args, "is:writelog", &level, &msg))
        return NULL;

    host_->writelog_(level, "%s", msg);
    Py_RETURN_NONE;
}

static PyObject*
pytcpconnect_(PyObject* self, PyObject* args)
{
    const char* sname, * host, * serv;
    int cid;

    if (!PyArg_ParseTuple(args, "sss:tcpconnect", &sname, &host, &serv))
        return NULL;

    if (-1 == (cid = host_->tcpconnect_(sname, host, serv))) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    return Py_BuildValue("i", cid);
}

static PyObject*
pytcplisten_(PyObject* self, PyObject* args)
{
    const char* sname, * host, * serv;

    if (!PyArg_ParseTuple(args, "sss:tcplisten", &sname, &host, &serv))
        return NULL;

    if (-1 == host_->tcplisten_(sname, host, serv)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject*
pypost_(PyObject* self, PyObject* args)
{
    const char* sname;
    int type;
    PyObject* user;

    if (!PyArg_ParseTuple(args, "siO:post", &sname, &type, &user))
        return NULL;

    if (-1 == host_->post_(sname, type, user)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    if (opencall_)
        pushuser_(user);
    Py_INCREF(user);
    Py_RETURN_NONE;
}

static PyObject*
pysettimer_(PyObject* self, PyObject* args)
{
    const char* sname;
    int tid;
    unsigned ms;
    PyObject* user;

    if (!PyArg_ParseTuple(args, "siIO:settimer", &sname, &tid, &ms, &user))
        return NULL;

    if (-1 == (tid = host_->settimer_(sname, tid, ms, user))) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    if (opencall_)
        pushuser_(user);
    Py_INCREF(user);
    return Py_BuildValue("i", tid);
}

static PyObject*
pyresettimer_(PyObject* self, PyObject* args)
{
    const char* sname;
    int tid;
    unsigned ms;

    if (!PyArg_ParseTuple(args, "siI:resettimer", &sname, &tid, &ms))
        return NULL;

    if (-1 == host_->resettimer_(sname, tid, ms)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject*
pycanceltimer_(PyObject* self, PyObject* args)
{
    const char* sname;
    int tid;

    if (!PyArg_ParseTuple(args, "si:canceltimer", &sname, &tid))
        return NULL;

    if (-1 == host_->canceltimer_(sname, tid)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject*
pyshutdown_(PyObject* self, PyObject* args)
{
    int cid;

    if (!PyArg_ParseTuple(args, "i:shutdown", &cid))
        return NULL;

    if (-1 == host_->shutdown_(cid)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject*
pysend_(PyObject* self, PyObject* args)
{
    const char* sname;
    int cid;
    const char* buf;
    int size;
    unsigned flags;

    if (!PyArg_ParseTuple(args, "sis#I:send", &sname, &cid, &buf, &size,
                          &flags))
        return NULL;

    if (-1 == host_->send_(sname, cid, buf, size, flags)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject*
pysetrwtimer_(PyObject* self, PyObject* args)
{
    int cid;
    unsigned ms, flags;

    if (!PyArg_ParseTuple(args, "iII:setrwtimer", &cid, &ms, &flags))
        return NULL;

    if (-1 == host_->setrwtimer_(cid, ms, flags)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject*
pyresetrwtimer_(PyObject* self, PyObject* args)
{
    int cid;
    unsigned ms, flags;

    if (!PyArg_ParseTuple(args, "iII:resetrwtimer", &cid, &ms, &flags))
        return NULL;

    if (-1 == host_->resetrwtimer_(cid, ms, flags)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject*
pycancelrwtimer_(PyObject* self, PyObject* args)
{
    int cid;
    unsigned flags;
    if (!PyArg_ParseTuple(args, "iI:cancelrwtimer", &cid, &flags))
        return NULL;

    if (-1 == host_->cancelrwtimer_(cid, flags)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    Py_RETURN_NONE;
}

/**
  string getenv (string sname, string name);
  void writelog (int level, string msg);
  int tcpconnect (string sname, string host, string serv);
  void tcplisten (string sname, string host, string serv);
  void post (string sname, int type, object user);
  int settimer (string sname, int tid, unsigned ms, object user);
  void resettimer (string sname, int tid, unsigned ms);
  void canceltimer (string sname, int tid);
  void shutdown (int cid);
  void send (string sname, int cid, buffer buf);
  void setrwtimer (int cid, unsigned ms, unsigned flags);
  void resetrwtimer (int cid, unsigned ms, unsigned flags);
  void cancelrwtimer (int cid, unsigned flags);
*/

static PyMethodDef pymethods_[] = {
    {
        "getenv", pygetenv_, METH_VARARGS,
        "TODO"
    },
    {
        "writelog", pywritelog_, METH_VARARGS,
        "TODO"
    },
    {
        "tcpconnect", pytcpconnect_, METH_VARARGS,
        "TODO"
    },
    {
        "tcplisten", pytcplisten_, METH_VARARGS,
        "TODO"
    },
    {
        "post", pypost_, METH_VARARGS,
        "TODO"
    },
    {
        "settimer", pysettimer_, METH_VARARGS,
        "TODO"
    },
    {
        "resettimer", pyresettimer_, METH_VARARGS,
        "TODO"
    },
    {
        "canceltimer", pycanceltimer_, METH_VARARGS,
        "TODO"
    },
    {
        "shutdown", pyshutdown_, METH_VARARGS,
        "TODO"
    },
    {
        "send", pysend_, METH_VARARGS,
        "TODO"
    },
    {
        "setrwtimer", pysetrwtimer_, METH_VARARGS,
        "TODO"
    },
    {
        "resetrwtimer", pyresetrwtimer_, METH_VARARGS,
        "TODO"
    },
    {
        "cancelrwtimer", pycancelrwtimer_, METH_VARARGS,
        "TODO"
    },
    { NULL, NULL, 0, NULL }
};

static void
pyfree_(void)
{
    if (Py_IsInitialized())
        Py_Finalize();
}

static int
pycreate_(void)
{
    Py_Initialize();
    if (!(pyaugas_ = Py_InitModule("augas", pymethods_)))
        goto fail;

    PyModule_AddIntConstant(pyaugas_, "LOGCRIT", AUGAS_LOGCRIT);
    PyModule_AddIntConstant(pyaugas_, "LOGERROR", AUGAS_LOGERROR);
    PyModule_AddIntConstant(pyaugas_, "LOGWARN", AUGAS_LOGWARN);
    PyModule_AddIntConstant(pyaugas_, "LOGNOTICE", AUGAS_LOGNOTICE);
    PyModule_AddIntConstant(pyaugas_, "LOGINFO", AUGAS_LOGINFO);
    PyModule_AddIntConstant(pyaugas_, "LOGDEBUG", AUGAS_LOGDEBUG);

    PyModule_AddIntConstant(pyaugas_, "TIMRD", AUGAS_TIMRD);
    PyModule_AddIntConstant(pyaugas_, "TIMWR", AUGAS_TIMWR);
    PyModule_AddIntConstant(pyaugas_, "TIMBOTH", AUGAS_TIMBOTH);

    PyModule_AddIntConstant(pyaugas_, "SNDSELF", AUGAS_SNDSELF);
    PyModule_AddIntConstant(pyaugas_, "SNDOTHER", AUGAS_SNDOTHER);
    PyModule_AddIntConstant(pyaugas_, "SNDALL", AUGAS_SNDALL);

    return 0;

 fail:
    pyfree_();
    return -1;
}

static void
closesess_(const struct augas_sess* sess)
{
    struct import_* import = sess->user_;
    assert(sess->user_);

    host_->writelog_(AUGAS_LOGINFO, "closesess_()");
    freeimport_(import);
}

static int
opensess_(struct augas_sess* sess)
{
    struct import_* import;

    host_->writelog_(AUGAS_LOGINFO, "opensess_()");
    if (!(import = createimport_(sess->name_)))
        return -1;

    sess->user_ = import;
    return 0;
}

static int
event_(const struct augas_sess* sess, int type, void* user)
{
    struct import_* import = sess->user_;
    PyObject* x = user;
    int ret = 0;
    assert(sess->user_);
    assert(user);

    host_->writelog_(AUGAS_LOGINFO, "event_()");
    if (import->event_) {

        PyObject* y = PyObject_CallFunction(import->event_, "siO",
                                            sess->name_, type, x);

        if (0 == (ret = check_(y))) {
            Py_DECREF(y);
        }
    }

    Py_DECREF(x);
    return ret;
}

static int
expire_(const struct augas_sess* sess, int tid, void* user, unsigned* ms)
{
    struct import_* import = sess->user_;
    int ret = 0;
    assert(sess->user_);
    assert(user);

    host_->writelog_(AUGAS_LOGINFO, "expire_()");
    if (import->expire_) {

        PyObject* x = user;
        PyObject* y = PyInt_FromLong(*ms);
        PyObject* z = PyObject_CallFunction(import->expire_, "siOO",
                                            sess->name_, tid, x, y);
        if (0 == (ret = check_(z))) {
            *ms = PyInt_AsLong(y);
            Py_DECREF(z);
        }

        Py_DECREF(y);

        if (0 == *ms) {
            Py_DECREF(x);
        }
    }
    return ret;
}

static int
reconf_(const struct augas_sess* sess)
{
    struct import_* import = sess->user_;
    int ret = 0;
    assert(sess->user_);

    host_->writelog_(AUGAS_LOGINFO, "reconf_()");
    if (import->reconf_) {

        PyObject* x = PyObject_CallFunction(import->reconf_, "s",
                                            sess->name_);

        if (0 == (ret = check_(x))) {
            Py_DECREF(x);
        }
    }
    return ret;
}

static void
closeconn_(const struct augas_conn* conn)
{
    struct import_* import = conn->sess_->user_;
    PyObject* x = conn->user_;
    assert(conn->user_);
    assert(conn->sess_->user_);

    host_->writelog_(AUGAS_LOGINFO, "closeconn_()");
    if (import->closeconn_) {

        PyObject* y = PyObject_CallFunction(import->closeconn_, "siO",
                                            conn->sess_->name_, conn->id_, x);

        if (0 == check_(y)) {
            Py_DECREF(y);
        }
    }
    Py_DECREF(x);
}

static int
openconn_(struct augas_conn* conn, const char* addr, unsigned short port)
{
    struct import_* import = conn->sess_->user_;
    PyObject* x;
    int ret = 0;
    assert(conn->sess_->user_);

    host_->writelog_(AUGAS_LOGINFO, "openconn_()");
    if (import->openconn_) {
        opencall_ = 1;
        x = PyObject_CallFunction(import->openconn_, "sisH",
                                  conn->sess_->name_, conn->id_, addr, port);
        opencall_ = 0;
        if (0 == (ret = check_(x)))
            retain_();
        else
            release_();
    } else {
        x = Py_None;
        Py_INCREF(x);
    }

    conn->user_ = x;
    return ret;
}

static void
notconn_(const struct augas_conn* conn)
{
    struct import_* import = conn->sess_->user_;
    PyObject* x = conn->user_;
    assert(conn->user_);
    assert(conn->sess_->user_);

    host_->writelog_(AUGAS_LOGINFO, "notconn_()");
    if (import->notconn_) {

        PyObject* y = PyObject_CallFunction(import->notconn_, "siO",
                                            conn->sess_->name_, conn->id_, x);

        if (0 == check_(y)) {
            Py_DECREF(y);
        }
    }
    Py_DECREF(x);
}

static int
data_(const struct augas_conn* conn, const char* buf, size_t size)
{
    struct import_* import = conn->sess_->user_;
    int ret = 0;
    assert(conn->user_);
    assert(conn->sess_->user_);

    host_->writelog_(AUGAS_LOGINFO, "data_()");
    if (import->data_) {

        PyObject* x = conn->user_;
        PyObject* y = PyBuffer_FromMemory((void*)buf, size);
        PyObject* z = PyObject_CallFunction(import->data_, "siOO",
                                            conn->sess_->name_, conn->id_, x,
                                            y);
        if (0 == (ret = check_(z))) {
            Py_DECREF(z);
        }
        Py_DECREF(y);
    }
    return ret;
}

static int
rdexpire_(const struct augas_conn* conn, unsigned* ms)
{
    struct import_* import = conn->sess_->user_;
    int ret = 0;
    assert(conn->user_);
    assert(conn->sess_->user_);

    host_->writelog_(AUGAS_LOGINFO, "rdexpire_()");
    if (import->rdexpire_) {

        PyObject* x = conn->user_;
        PyObject* y = PyInt_FromLong(*ms);
        PyObject* z = PyObject_CallFunction(import->rdexpire_, "siOO",
                                            conn->sess_->name_, conn->id_, x,
                                            y);

        if (0 == (ret = check_(z))) {
            *ms = PyInt_AsLong(y);
            Py_DECREF(z);
        }
        Py_DECREF(y);
    }

    return ret;
}

static int
wrexpire_(const struct augas_conn* conn, unsigned* ms)
{
    struct import_* import = conn->sess_->user_;
    int ret = 0;
    assert(conn->user_);
    assert(conn->sess_->user_);

    host_->writelog_(AUGAS_LOGINFO, "wrexpire_()");
    if (import->wrexpire_) {

        PyObject* x = conn->user_;
        PyObject* y = PyInt_FromLong(*ms);
        PyObject* z = PyObject_CallFunction(import->wrexpire_, "siOO",
                                            conn->sess_->name_, conn->id_, x,
                                            y);

        if (0 == (ret = check_(z))) {
            *ms = PyInt_AsLong(y);
            Py_DECREF(z);
        }
        Py_DECREF(y);
    }

    return ret;
}

static int
teardown_(const struct augas_conn* conn)
{
    struct import_* import = conn->sess_->user_;
    PyObject* x = conn->user_;
    int ret = 0;
    assert(conn->user_);
    assert(conn->sess_->user_);

    host_->writelog_(AUGAS_LOGINFO, "teardown_()");
    if (import->teardown_) {

        PyObject* y = PyObject_CallFunction(import->teardown_, "siO",
                                            conn->sess_->name_, conn->id_, x);

        if (0 == (ret = check_(y))) {
            Py_DECREF(y);
        }

    } else
        host_->shutdown_(conn->id_);

    return ret;
}

static const struct augas_module fntable_ = {
    closesess_,
    opensess_,
    event_,
    expire_,
    reconf_,
    closeconn_,
    openconn_,
    notconn_,
    data_,
    rdexpire_,
    wrexpire_,
    teardown_
};

static const struct augas_module*
load_(const char* name, const struct augas_host* host)
{
    /**
       Fail if module has already been loaded.
    */

    host->writelog_(AUGAS_LOGINFO, "loading modpython");

    if (host_)
        return NULL;

    host_ = host;

    if (pycreate_() < 0)
        return NULL;

    return &fntable_;
}

static void
unload_(void)
{
    host_->writelog_(AUGAS_LOGINFO, "unloading modpython");
    pyfree_();
    host_ = 0;
}

AUGAS_MODULE(load_, unload_)
