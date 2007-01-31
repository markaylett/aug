#if defined(_WIN32)
# undef _DEBUG
# include <direct.h>
#endif /* _WIN32 */

#include <Python.h>
#include <augas.h>

#include <stdlib.h>

struct import_ {
    PyObject* module_;
    PyObject* term_;
    PyObject* init_;
    PyObject* reconf_;
    PyObject* event_;
    PyObject* closed_;
    PyObject* teardown_;
    PyObject* accept_;
    PyObject* connected_;
    PyObject* data_;
    PyObject* rdexpire_;
    PyObject* wrexpire_;
    PyObject* expire_;
    int open_;
};

static const struct augas_host* host_ = NULL;
static PyObject* pyaugas_ = NULL;
static int refs_ = 0;

static void
decuser_(PyObject* user, const char* func)
{
    --refs_;
    if (host_)
        host_->writelog_
            (AUGAS_LOGDEBUG,
             "releasing python object: user=[%08x], fn=[%s()], refs=[%d]",
             user, func, refs_);
    Py_DECREF(user);
}

static void
incuser_(PyObject* user, const char* func)
{
    ++refs_;
    host_->writelog_
        (AUGAS_LOGDEBUG,
         "retaining python object: user=[%08x], fn=[%s()], refs=[%d]",
         user, func, refs_);
    Py_INCREF(user);
}

static void
noincuser_(PyObject* user, const char* func)
{
    ++refs_;
    host_->writelog_
        (AUGAS_LOGDEBUG,
         "retaining python object: user=[%08x], fn=[%s()], refs=[%d]",
         user, func, refs_);
}

static void
free_(void* user)
{
    decuser_(user, __func__);
}

static PyObject*
inctrue_(void)
{
    Py_INCREF(Py_True);
    return Py_True;
}

static PyObject*
incfalse_(void)
{
    Py_INCREF(Py_False);
    return Py_False;
}

static PyObject*
incnone_(void)
{
    Py_INCREF(Py_None);
    return Py_None;
}

static void
setpath_(void)
{
    const char* s;
    PyObject* sys;

    if ((s = host_->getenv_("rundir")))
        chdir(s);

    if (!(s = host_->getenv_("module.modpython.pythonpath")))
        s = "bin";
    else
        host_->writelog_(AUGAS_LOGDEBUG,
                         "module.modpython.pythonpath=[%s]", s);

    chdir(s);

    if ((sys = PyImport_ImportModule("sys"))) {

        PyObject* path = PyObject_GetAttrString(sys, "path");
        if (path) {

            char buf[1024];
            PyObject* dir;

            getcwd(buf, sizeof(buf));

            if ((dir = PyString_FromString(buf))) {

                host_->writelog_(AUGAS_LOGDEBUG, "adding to sys.path: %s",
                                 buf);

                PyList_Insert(path, 0, dir);

                Py_DECREF(dir);
            }

            Py_DECREF(path);
        }

        Py_DECREF(sys);
    }
}

static void
printerr_(void)
{
    PyObject* type, * value, * traceback;
    PyObject* module;

    if (!PyErr_Occurred())
        return;

    /* Returns owned references. */

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

    PyErr_Clear();
}

static PyObject*
getmethod_(PyObject* module, const char* name)
{
    PyObject* x = PyObject_GetAttrString(module, (char*)name);
    if (x) {
        if (!PyCallable_Check(x)) {
            Py_DECREF(x);
            x = NULL;
        }
    } else {
        host_->writelog_(AUGAS_LOGDEBUG, "no binding for %s()", name);
        PyErr_Clear();
    }
    return x;
}

static void
freeimport_(struct import_* import)
{
    if (import->open_ && import->term_) {

        const char* sname = PyModule_GetName(import->module_);
        if (sname) {
            PyObject* x = PyObject_CallFunction(import->term_, "s",
                                                sname);
            if (x) {
                Py_DECREF(x);
            } else
                printerr_();
        }
    }

    Py_XDECREF(import->teardown_);
    Py_XDECREF(import->wrexpire_);
    Py_XDECREF(import->rdexpire_);
    Py_XDECREF(import->data_);
    Py_XDECREF(import->accept_);
    Py_XDECREF(import->connected_);
    Py_XDECREF(import->closed_);
    Py_XDECREF(import->reconf_);
    Py_XDECREF(import->expire_);
    Py_XDECREF(import->event_);
    Py_XDECREF(import->init_);
    Py_XDECREF(import->term_);

    Py_XDECREF(import->module_);
    free(import);
}

static struct import_*
createimport_(const char* sname)
{
    struct import_* import = malloc(sizeof(struct import_));
    if (!import)
        return NULL;

    import->open_ = 0;
    if (!(import->module_ = PyImport_ImportModule((char*)sname))) {
        printerr_();
        goto fail;
    }

    import->term_ = getmethod_(import->module_, "term");
    import->init_ = getmethod_(import->module_, "init");
    import->event_ = getmethod_(import->module_, "event");
    import->expire_ = getmethod_(import->module_, "expire");
    import->reconf_ = getmethod_(import->module_, "reconf");
    import->closed_ = getmethod_(import->module_, "closed");
    import->accept_ = getmethod_(import->module_, "accept");
    import->connected_ = getmethod_(import->module_, "connected");
    import->data_ = getmethod_(import->module_, "data");
    import->rdexpire_ = getmethod_(import->module_, "rdexpire");
    import->wrexpire_ = getmethod_(import->module_, "wrexpire");
    import->teardown_ = getmethod_(import->module_, "teardown");

    return import;

 fail:
    freeimport_(import);
    return NULL;
}

static PyObject*
pywritelog_(PyObject* self, PyObject* args)
{
    int level;
    const char* msg;

    if (!PyArg_ParseTuple(args, "is:writelog", &level, &msg))
        return NULL;

    host_->writelog_(level, "%s", msg);
    return incnone_();
}

static PyObject*
pyreconf_(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ":reconf"))
        return NULL;

    host_->reconf_();
    return incnone_();
}

static PyObject*
pystop_(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ":stop"))
        return NULL;

    host_->stop_();
    return incnone_();
}

static PyObject*
pypost_(PyObject* self, PyObject* args)
{
    const char* sname;
    int type;
    PyObject* user;

    if (!PyArg_ParseTuple(args, "siO:post", &sname, &type, &user))
        return NULL;

    if (-1 == host_->post_(sname, type, user, free_)) {

        /* Examples show that PyExc_RuntimeError does not need to be
           Py_INCREF()-ed. */

        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    incuser_(user, __func__);
    return incnone_();
}

static PyObject*
pydelegate_(PyObject* self, PyObject* args)
{
    const char* sname;
    int type;
    PyObject* user;

    if (!PyArg_ParseTuple(args, "siO:delegate", &sname, &type, &user))
        return NULL;

    if (-1 == host_->delegate_(sname, type, user)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    return incnone_();
}

static PyObject*
pygetenv_(PyObject* self, PyObject* args)
{
    const char* name, * value;

    if (!PyArg_ParseTuple(args, "s:getenv", &name))
        return NULL;

    if (!(value = host_->getenv_(name)))
        return incnone_();

    return Py_BuildValue("s", value);
}

static PyObject*
pyshutdown_(PyObject* self, PyObject* args)
{
    int fid;

    if (!PyArg_ParseTuple(args, "i:shutdown", &fid))
        return NULL;

    if (-1 == host_->shutdown_(fid)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    return incnone_();
}

static PyObject*
pytcpconnect_(PyObject* self, PyObject* args)
{
    const char* sname, * host, * serv;
    PyObject* user;
    int cid;

    if (!PyArg_ParseTuple(args, "sssO:tcpconnect", &sname, &host, &serv,
                          &user))
        return NULL;

    if (-1 == (cid = host_->tcpconnect_(sname, host, serv, user))) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    /* When tcpconnect() becomes asynchronous, user will need to be
       Py_INCREF()-ed. */

    incuser_(user, __func__);
    return Py_BuildValue("i", cid);
}

static PyObject*
pytcplisten_(PyObject* self, PyObject* args)
{
    const char* sname, * host, * serv;
    PyObject* user;
    int lid;

    if (!PyArg_ParseTuple(args, "sssO:tcplisten", &sname, &host, &serv,
                          &user))
        return NULL;

    if (-1 == (lid = host_->tcplisten_(sname, host, serv, user))) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    incuser_(user, __func__);
    return Py_BuildValue("i", lid);
}

static PyObject*
pysend_(PyObject* self, PyObject* args)
{
    int cid;
    const char* buf;
    int size;

    if (!PyArg_ParseTuple(args, "is#:send", &cid, &buf, &size))
        return NULL;

    if (-1 == host_->send_(cid, buf, size)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    return incnone_();
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

    return incnone_();
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

    return incnone_();
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

    return incnone_();
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

    if (-1 == (tid = host_->settimer_(sname, tid, ms, user, free_))) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    incuser_(user, __func__);
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

    switch (host_->resettimer_(sname, tid, ms)) {
    case -1:
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    case AUGAS_NONE:
        return incfalse_();
    }

    return inctrue_();
}

static PyObject*
pycanceltimer_(PyObject* self, PyObject* args)
{
    const char* sname;
    int tid;

    if (!PyArg_ParseTuple(args, "si:canceltimer", &sname, &tid))
        return NULL;

    switch (host_->canceltimer_(sname, tid)) {
    case -1:
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    case AUGAS_NONE:
        return incfalse_();
    }

    return inctrue_();
}

static PyMethodDef pymethods_[] = {
    {
        "writelog", pywritelog_, METH_VARARGS,
        "TODO"
    },
    {
        "reconf", pyreconf_, METH_VARARGS,
        "TODO"
    },
    {
        "stop", pystop_, METH_VARARGS,
        "TODO"
    },
    {
        "post", pypost_, METH_VARARGS,
        "TODO"
    },
    {
        "delegate", pydelegate_, METH_VARARGS,
        "TODO"
    },
    {
        "getenv", pygetenv_, METH_VARARGS,
        "TODO"
    },
    {
        "shutdown", pyshutdown_, METH_VARARGS,
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
    { NULL, NULL, 0, NULL }
};

static void
pyfree_(void)
{
    if (Py_IsInitialized()) {

        host_->writelog_(AUGAS_LOGDEBUG, "finalising python interpreter");
        Py_Finalize();

        if (refs_)
            host_->writelog_(AUGAS_LOGERROR,
                             "unreleased python references: refs=[%d]",
                             refs_);
    }
}

static int
pycreate_(void)
{
    host_->writelog_(AUGAS_LOGDEBUG, "initialising python interpreter");
    Py_Initialize();
    /* Py_VerboseFlag = 1; */
    setpath_();

    /* Returns borrowed reference. */

    host_->writelog_(AUGAS_LOGDEBUG, "initialising augas module");
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

    return 0;

 fail:
    pyfree_();
    return -1;
}

static void
term_(const struct augas_sess* sess)
{
    struct import_* import = sess->user_;
    assert(sess->user_);
    freeimport_(import);
}

static int
init_(struct augas_sess* sess)
{
    struct import_* import;

    if (!(import = createimport_(sess->name_)))
        return -1;

    sess->user_ = import;

    if (import->init_) {

        PyObject* x = PyObject_CallFunction(import->init_, "s",
                                            sess->name_);
        if (!x) {
            printerr_();
            freeimport_(import);
            return -1;
        }
        Py_DECREF(x);
    }

    import->open_ = 1;
    return 0;
}

static int
reconf_(const struct augas_sess* sess)
{
    struct import_* import = sess->user_;
    int ret = 0;
    assert(sess->user_);

    if (import->reconf_) {

        PyObject* x = PyObject_CallFunction(import->reconf_, "s",
                                            sess->name_);
        if (x) {
            Py_DECREF(x);
        } else {
            printerr_();
            ret = -1;
        }
    }
    return ret;
}

static int
event_(const struct augas_sess* sess, int type, void* user)
{
    struct import_* import = sess->user_;
    PyObject* x = user;
    int ret = 0;
    assert(sess->user_);
    assert(user);

    if (import->event_) {

        PyObject* y = PyObject_CallFunction(import->event_, "siO",
                                            sess->name_, type, x);
        if (y) {
            Py_DECREF(y);
        } else {
            printerr_();
            ret = -1;
        }
    }

    /* x will be Py_DECREF()-ed by free_(). */

    return ret;
}

static void
closed_(const struct augas_object* sock)
{
    struct import_* import = sock->sess_->user_;
    PyObject* x = sock->user_;
    assert(sock->user_);
    assert(sock->sess_->user_);

    if (import->closed_) {

        PyObject* y = PyObject_CallFunction(import->closed_, "siO",
                                            sock->sess_->name_, sock->id_, x);
        if (y) {
            Py_DECREF(y);
        } else
            printerr_();
    }
    decuser_(x, __func__);
}

static int
teardown_(const struct augas_object* sock)
{
    struct import_* import = sock->sess_->user_;
    PyObject* x = sock->user_;
    int ret = 0;
    assert(sock->user_);
    assert(sock->sess_->user_);

    if (import->teardown_) {

        PyObject* y = PyObject_CallFunction(import->teardown_, "siO",
                                            sock->sess_->name_, sock->id_, x);
        if (y) {
            Py_DECREF(y);
        } else {
            printerr_();
            ret = -1;
        }

    } else
        host_->shutdown_(sock->id_);

    return ret;
}

static int
accept_(struct augas_object* sock, const char* addr, unsigned short port)
{
    struct import_* import = sock->sess_->user_;
    PyObject* x = sock->user_;
    assert(sock->user_);
    assert(sock->sess_->user_);

    if (import->accept_) {

        PyObject* y = PyObject_CallFunction(import->accept_, "siOsH",
                                            sock->sess_->name_, sock->id_, x,
                                            addr, port);
        if (!y) {

            /* closed() will not be called if accept() fails. */

            printerr_();
            return -1;

        } else if (PyBool_Check(y)) {

            if (y == Py_False)
                return -1;

            /* x unchanged. */

            Py_INCREF(x);

        } else
            x = y;

    } else {

        /* x unchanged. */

        Py_INCREF(x);
    }

    sock->user_ = x;
    noincuser_(x, __func__);

    /* Original x is still retained by the listener. */

    return 0;
}

static int
connected_(struct augas_object* sock, const char* addr, unsigned short port)
{
    struct import_* import = sock->sess_->user_;
    PyObject* x = sock->user_, * y = NULL;
    int ret = 0;
    assert(sock->user_);
    assert(sock->sess_->user_);

    if (import->connected_) {

        if (!(y = PyObject_CallFunction(import->connected_, "siOsH",
                                        sock->sess_->name_, sock->id_, x,
                                        addr, port))) {
            printerr_();
            ret = -1;
        }
    }

    /* closed() will always be called, even if connected() fails. */

    if (!y) {
        y = x;
        Py_INCREF(y);
    }

    sock->user_ = y;
    noincuser_(y, __func__);
    decuser_(x, __func__);
    return ret;
}

static int
data_(const struct augas_object* sock, const char* buf, size_t size)
{
    struct import_* import = sock->sess_->user_;
    int ret = 0;
    assert(sock->user_);
    assert(sock->sess_->user_);

    if (import->data_) {

        PyObject* x = sock->user_;
        PyObject* y = PyBuffer_FromMemory((void*)buf, size);
        PyObject* z = PyObject_CallFunction(import->data_, "siOO",
                                            sock->sess_->name_, sock->id_, x,
                                            y);
        if (z) {
            Py_DECREF(z);
        } else {
            printerr_();
            ret = -1;
        }

        Py_DECREF(y);
    }
    return ret;
}

static int
rdexpire_(const struct augas_object* sock, unsigned* ms)
{
    struct import_* import = sock->sess_->user_;
    int ret = 0;
    assert(sock->user_);
    assert(sock->sess_->user_);

    if (import->rdexpire_) {

        PyObject* x = sock->user_;
        PyObject* y = PyInt_FromLong(*ms);
        PyObject* z = PyObject_CallFunction(import->rdexpire_, "siOO",
                                            sock->sess_->name_, sock->id_, x,
                                            y);
        if (z) {
            if (PyInt_Check(z))
                *ms = PyInt_AsLong(z);
            Py_DECREF(z);
        } else {
            printerr_();
            ret = -1;
        }

        Py_DECREF(y);
    }

    return ret;
}

static int
wrexpire_(const struct augas_object* sock, unsigned* ms)
{
    struct import_* import = sock->sess_->user_;
    int ret = 0;
    assert(sock->user_);
    assert(sock->sess_->user_);

    if (import->wrexpire_) {

        PyObject* x = sock->user_;
        PyObject* y = PyInt_FromLong(*ms);
        PyObject* z = PyObject_CallFunction(import->wrexpire_, "siOO",
                                            sock->sess_->name_, sock->id_, x,
                                            y);
        if (z) {
            if (PyInt_Check(z))
                *ms = PyInt_AsLong(z);
            Py_DECREF(z);
        } else {
            printerr_();
            ret = -1;
        }

        Py_DECREF(y);
    }

    return ret;
}

static int
expire_(const struct augas_object* timer, unsigned* ms)
{
    struct import_* import = timer->sess_->user_;
    int ret = 0;
    assert(timer->sess_->user_);
    assert(timer->user_);

    if (import->expire_) {

        PyObject* x = timer->user_;
        PyObject* y = PyInt_FromLong(*ms);
        PyObject* z = PyObject_CallFunction(import->expire_, "siOO",
                                            timer->sess_->name_, timer->id_,
                                            x, y);
        Py_DECREF(y);

        if (z) {
            if (PyInt_Check(z))
                *ms = PyInt_AsLong(z);
            Py_DECREF(z);
        } else {
            printerr_();
            ret = -1;
        }

        /* x will be Py_DECREF()-ed by free_() when *ms == 0. */
    }
    return ret;
}

static const struct augas_module fntable_ = {
    term_,
    init_,
    reconf_,
    event_,
    closed_,
    teardown_,
    accept_,
    connected_,
    data_,
    rdexpire_,
    wrexpire_,
    expire_
};

static const struct augas_module*
load_(const char* name, const struct augas_host* host)
{
    /* Fail if module has already been loaded. */

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
