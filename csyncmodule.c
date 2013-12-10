#include <Python.h>
#include <structmember.h>
#include <csync/csync.h>
#undef NDEBUG
#include <assert.h>

/*
#include "csync_private.h"
ctx->status & CSYNC_STATUS_INIT

If we follow python naming convention:
CSyncObject
CSync_Type

PyErr_WriteUnraisable

TODO:
enum csync_notify_type_e
enum csync_status_codes_e
enum csync_instructions_e

django.get_version()    # '1.6'
django.VERSION          # (1, 6, 0, 'final', 0)

const char *errstr = csync_get_status_string (self->ctx); // always null?
*/

#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof((arr)[0]))


// Forward decleare
//typedef struct CSync;
struct _CSyncObject;
typedef struct _CSyncObject CSync;

static PyObject* throw_csync_error (CSync*);


/** Global variables / state */
static struct {
    PyObject *log_callback;
//} globals;
} globals = {0};    // zero init. (not needed; compiler shall do it)



/**
 * The CSync Python type.
 */

struct _CSyncObject {
    PyObject_HEAD
    // Type-specific fields go here:
    CSYNC *ctx;
    struct {
        PyObject *auth;
        PyObject *tree_walk;
        PyObject *file_progress;
        PyObject *overall_progress;
    } callbacks;
};

// Object struct
/*
typedef struct {
    PyObject_HEAD
    // Type-specific fields go here:
    CSYNC *ctx;
    struct {
        PyObject *auth;
        PyObject *tree_walk;
        PyObject *file_progress;
        PyObject *overall_progress;
    } callbacks;
} CSync;
//} CSyncObject;
*/



/** Exception and error handling. */

static PyObject *CSyncError;

static struct {
    int code;
    const char* message;
} error_strings[] = {
    { CSYNC_STATUS_OK,                  "OK" },
    { CSYNC_STATUS_ERROR,               "ERROR" },
    { CSYNC_STATUS_UNSUCCESSFUL,        "UNSUCCESSFUL" },
    { CSYNC_STATUS_NO_LOCK,             "NO_LOCK" },
    { CSYNC_STATUS_STATEDB_LOAD_ERROR,  "STATEDB_LOAD_ERROR" },
    { CSYNC_STATUS_STATEDB_WRITE_ERROR, "STATEDB_WRITE_ERROR" },
    { CSYNC_STATUS_NO_MODULE,           "NO_MODULE" },
    { CSYNC_STATUS_TIMESKEW,            "TIMESKEW" },
    { CSYNC_STATUS_FILESYSTEM_UNKNOWN,  "FILESYSTEM_UNKNOWN" },
    { CSYNC_STATUS_TREE_ERROR,          "TREE_ERROR" },
    { CSYNC_STATUS_MEMORY_ERROR,        "MEMORY_ERROR" },
    { CSYNC_STATUS_PARAM_ERROR,         "PARAM_ERROR" },
    { CSYNC_STATUS_UPDATE_ERROR,        "UPDATE_ERROR" },
    { CSYNC_STATUS_RECONCILE_ERROR,     "RECONCILE_ERROR" },
    { CSYNC_STATUS_PROPAGATE_ERROR,     "PROPAGATE_ERROR" },
    { CSYNC_STATUS_REMOTE_ACCESS_ERROR, "REMOTE_ACCESS_ERROR" },
    { CSYNC_STATUS_REMOTE_CREATE_ERROR, "REMOTE_CREATE_ERROR" },
    { CSYNC_STATUS_REMOTE_STAT_ERROR,   "REMOTE_STAT_ERROR" },
    { CSYNC_STATUS_LOCAL_CREATE_ERROR,  "LOCAL_CREATE_ERROR" },
    { CSYNC_STATUS_LOCAL_STAT_ERROR,    "LOCAL_STAT_ERROR" },
    { CSYNC_STATUS_PROXY_ERROR,         "PROXY_ERROR" },
    { CSYNC_STATUS_LOOKUP_ERROR,        "LOOKUP_ERROR" },
    { CSYNC_STATUS_SERVER_AUTH_ERROR,   "SERVER_AUTH_ERROR" },
    { CSYNC_STATUS_PROXY_AUTH_ERROR,    "PROXY_AUTH_ERROR" },
    { CSYNC_STATUS_CONNECT_ERROR,       "CONNECT_ERROR" },
    { CSYNC_STATUS_TIMEOUT,             "TIMEOUT" },
    { CSYNC_STATUS_HTTP_ERROR,          "HTTP_ERROR" },
    { CSYNC_STATUS_PERMISSION_DENIED,   "PERMISSION_DENIED" },
    { CSYNC_STATUS_NOT_FOUND,           "NOT_FOUND" },
    { CSYNC_STATUS_FILE_EXISTS,         "FILE_EXISTS" },
    { CSYNC_STATUS_OUT_OF_SPACE,        "OUT_OF_SPACE" },
    { CSYNC_STATUS_QUOTA_EXCEEDED,      "QUOTA_EXCEEDED" },
    { CSYNC_STATUS_SERVICE_UNAVAILABLE, "SERVICE_UNAVAILABLE" },
    { CSYNC_STATUS_FILE_SIZE_ERROR,     "FILE_SIZE_ERROR" },
    { CSYNC_STATUS_CONTEXT_LOST,        "CONTEXT_LOST" },
    { CSYNC_STATUS_MERGE_FILETREE_ERROR,"MERGE_FILETREE_ERROR" },
    { CSYNC_STATUS_CSYNC_STATUS_ERROR,  "CSYNC_STATUS_ERROR" },
    { CSYNC_STATUS_OPENDIR_ERROR,       "OPENDIR_ERROR" },
    { CSYNC_STATUS_READDIR_ERROR,       "READDIR_ERROR" },
    { CSYNC_STATUS_OPEN_ERROR,          "OPEN_ERROR" },
};


static PyObject *
throw_csync_error (CSync *self)
{
#ifndef HAVE_CSYNC_STATUS_CODE
    PyErr_SetString (CSyncError, "Something went terribly wrong!");
    return NULL;
#endif
    int sc = csync_get_status_code (self->ctx);
    //assert (sc != CSYNC_STATUS_OK);
    size_t i;
    const char* str = "(unknown error)";
    for (i=0; i<sizeof(error_strings)/sizeof(error_strings[0]); ++i) {
        if (error_strings[i].code == sc) {
            str = error_strings[i].message;
            break;
        }
    }
    // XXX: Setting attribute on global exception object. This can't
    //      be thread safe, right!?!
    PyObject_SetAttrString (CSyncError, "status", PyInt_FromLong(sc));
    return PyErr_Format (CSyncError, "%s", str);
//    return PyErr_Format (err, "libcsync status code: %d", sc);
}



/**
 * Functions that Python requires to implement own type.
 */

// Constructor (allocate)
// @todo check if ctor args is passed in args
static PyObject *
_py_csync_new (PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CSync *self;

    puts ("XXX: WHY IS THIS NEVER CALLED?");

    self = (CSync*) type->tp_alloc (type, 0);
    if (! self)
        return NULL;

    // clear all members
    self->ctx = NULL;
    memset (&self->callbacks, 0, sizeof(self->callbacks));

    return (PyObject*) self;
}


// Constructor (initialize / construct)
// Q: object will not be constructed if this fails with -1?
// A: yes. but must set error, else:
//         SystemError: NULL result without error in PyObject_Call
static int
_py_csync_init (CSync *self, PyObject *args, PyObject *kwargs)
{
//    static char *kwlist[] = { "local", "remote", NULL};
    static const char *kwlist[] = { "local", "remote", NULL};
    const char *local, *remote;

//    if (! PyArg_ParseTupleAndKeywords (args, kwargs, "|ss",
//                                       kwlist, &local, &remote))
    if (! PyArg_ParseTupleAndKeywords (args, kwargs, "|ss",
                                       (char**)kwlist, &local, &remote))
        return -1;

    assert (self->ctx == NULL);
    if (csync_create (&self->ctx, local, remote) != 0)
        return -1;

    int rv;
    rv = csync_set_userdata (self->ctx, self);
    assert (rv==0);

    return 0;
    // @todo must free local&remote? (or keep'em on self)
    // A: no, ParseTuple returns borrowed references
}


// Destructor
static void
_py_csync_dealloc (CSync *self)
{
    assert (self->ctx);
    int rv = csync_destroy (self->ctx);
    assert (rv==0);
    // Q: howto report error? throw exception
    // A? PyErr_SetString(PyExc_AttributeError, "first");

    self->ob_type->tp_free ((PyObject*) self);
}



/**
 * Callback wrappers.
 */

static void
_py_log_callback_wrapper (int verbosity,
                          const char *function,
                          const char *buffer,
                          void *userdata)
{
    PyObject *cb_args;
    PyObject *result;

    cb_args = Py_BuildValue ("iss", verbosity, function, buffer);
    assert (cb_args);   // @todo pass exception back to interpereter. but how??

    result = PyObject_CallObject (globals.log_callback, cb_args);
    Py_DECREF (cb_args);
    if (! result) {
        fprintf (stderr, "Python error in callback. No way to abort csync!\n");
        // @todo PyErr_Print() before aborting
        abort();
        // Possible fix
        // 1) Use setjmp/longjump to do a non-local exit?
        // 2) Own csync error value to report this?
        //    I.e., callback returns FIRST_USER_ERROR+N, then csync
        //    return negative error that can be mapped back to user
        //    error code (N).
    }

    if (result != Py_None) {
        PyErr_Warn (PyExc_RuntimeWarning,
                    "Ignoring return value from 'log_handler. "
                    "It should return None");
    }
    Py_DECREF (result);
}


// Python callback is passed: (string prompt, bool echo, bool verify)
// And returns password string on success, or None on failure.
// Q: what to return?
static int
_py_auth_callback_wrapper (const char *prompt,
                           char       *buffer,
                           size_t      buffer_len,
                           int         echo,
                           int         verify,
                           void       *userdata)
{
    CSync *self;
    PyObject *arglist;
    PyObject *result;
//    printf ("  echo:\t %d\n", echo);
//    printf ("  verify:%d\n", verify);

    self = (CSync*) userdata;
    assert (self);

    arglist = Py_BuildValue ("(sii)", prompt, echo, verify);
    result = PyObject_CallObject (self->callbacks.auth, arglist);
    Py_XDECREF (arglist);
    if (! result) {
        // Python error / exception in callback
        // howto pass back to interpereter
        // PyWarn?
        fprintf (stderr, "\nFIXME Python error in auth-callback"
                         "\nFIXME Do not know how to handle that ...\n");
        assert (false);
    }

    if (result == Py_None)
        assert(0);
    if (! PyString_Check (result))
        assert(0);

    int rv;
    char *strbuf;
    Py_ssize_t sz;
    rv = PyString_AsStringAndSize (result, &strbuf, &sz);
    assert (rv==0);
    assert (sz < buffer_len);
    if (sz > 0)
        strcpy (buffer, strbuf);

    Py_DECREF (result);
    return 0;
}


static int
_py_treewalk_visit_func (TREE_WALK_FILE *file, void *userdata)
{
    CSync *self = (CSync*) userdata;
    PyObject *cb_args;
    PyObject *result;

    cb_args = Py_BuildValue ("sdIIIii",
                             file->path,
                             (double) file->modtime,
                             (unsigned int) file->uid,
                             (unsigned int) file->gid,
                             (unsigned int) file->mode,
                             file->type,
                             file->instruction);
    if (! cb_args) return -1;

    result = PyObject_CallObject (self->callbacks.tree_walk, cb_args);
    Py_DECREF (cb_args);
    if (! result) return -1;  // abort

    // @todo handle result
    Py_DECREF (result);

    // returning a negative number makes csync_walk_local_tree()
    // return -1. Q: Howto detect csync internal error vs visitor function
    // returning error?
    return 0;
    // @todo return instruction of what to do with the file?
}


static PyObject *
_py_csync_walk_tree (CSync *self, PyObject *args,
                     int (*walk_func) (CSYNC*, csync_treewalk_visit_func*, int))
{
    PyObject *visitor;
    int filter;
    int rv;

    if (! PyArg_ParseTuple (args, "Oi", &visitor, &filter))
        return NULL;
    if (! PyCallable_Check (visitor)) {
        PyErr_SetString (PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

    self->callbacks.tree_walk = visitor;
    rv = walk_func (self->ctx, _py_treewalk_visit_func, filter);
//    printf ("visitor returns: %d\n", rv);
    self->callbacks.tree_walk = NULL;   // borrowed reference

    // @todo rv==-1 can mean both csync error or error in python callback
    if (! CSYNC_STATUS_IS_OK (rv))
        return NULL;    // pass error back

    Py_RETURN_NONE;
}


// @todo test!
// @todo change assert(0) into aborting callback and pass error back to python.
//       but how? need csync c api support
static void
_py_overall_progress_wrapper (const char *file_name,
                              int file_no,
                              int file_cnt,
                              long long o1,
                              long long o2,
                              void *userdata)
{
//    puts ("overall-progress");
    CSync *self = (CSync*) userdata;
    PyObject *cb_args;
    PyObject *result;

    // Note: L (long) [PY_LONG_LONG] not available on all platforms.
    cb_args = Py_BuildValue ("siiLL", file_name, file_no, file_cnt, o1, o2);
    if (! cb_args) assert(0);

    result = PyObject_CallObject (self->callbacks.overall_progress, cb_args);
    Py_DECREF (cb_args);
    if (! result) assert(0);
    Py_DECREF (result);     // @todo warn if no Py_None?
}


// @todo test!
// @todo same issue as _py_overall_progress_wrapper
static void
_py_file_progress_wrapper (const char *remote_url,
                           enum csync_notify_type_e kind,
                           long long o1,
                           long long o2,
                           void *userdata)
{
    CSync *self = (CSync*) userdata;
    PyObject *cb_args;
    PyObject *result;

    // Note: L (long) [PY_LONG_LONG] not available on all platforms.
    cb_args = Py_BuildValue ("siLL", remote_url, kind, o1, o2);
    if (! cb_args) assert(0);

    result = PyObject_CallObject (self->callbacks.file_progress, cb_args);
    Py_DECREF (cb_args);
    if (! result) assert(0);
    Py_DECREF (result);     // @todo warn if no Py_None?
}


// helper containing common code to set callbacks
// @todo unset callback if None is passed
static int
_py_set_callback (PyObject *args, PyObject **slot)
{
    PyObject *tmp;
    if (! PyArg_ParseTuple (args, "O", &tmp))
        return -1;
    if (! PyCallable_Check (tmp)) {
        PyErr_SetString (PyExc_TypeError, "parameter must be callable");
        return -1;
    }

    Py_INCREF (tmp);
    Py_XDECREF (*slot);
    *slot = tmp;
    return 0;
}



/**
 * CSync methods.
 */

// just for testing
static PyObject *
py_csync_test (CSync *self, PyObject *args)
{
    Py_RETURN_NONE;
}


static PyObject *
py_csync_set_overall_progress_callback (CSync *self, PyObject *args)
{
    if (_py_set_callback (args, &self->callbacks.overall_progress) < 0)
        return NULL;
    int rv = csync_set_overall_progress_callback (self->ctx, _py_overall_progress_wrapper);
    assert (rv==0);
    Py_RETURN_NONE;
}


static PyObject *
py_csync_set_file_progress_callback (CSync *self, PyObject *args)
{
    if (_py_set_callback (args, &self->callbacks.file_progress) < 0)
        return NULL;
    int rv = csync_set_file_progress_callback (self->ctx, _py_file_progress_wrapper);
    assert (rv==0);
    Py_RETURN_NONE;
//    return _py_set_callback (args,
//                             csync_set_file_progress_callback,
//                             &self->callbacks.file_progress,
//                             _py_file_progress_wrapper);
}


#ifdef WITH_ICONV
static PyObject *
py_csync_set_iconv_codec (CSync *self, PyObject *args)
{
    const char *from;
    if (! PyArg_ParseTuple (args, "s", &from))
        return NULL;

    int rv = csync_set_iconv_codec (from);
    assert (rv==0);     // @note returns iconv error number
    Py_RETURN_NONE;
}
#endif


static PyObject *
py_csync_get_status_string (CSync *self)
{
    return Py_BuildValue ("s", csync_get_status_string (self->ctx));
//    const char* str;
//    str = csync_get_status_string (self->ctx);
//    return Py_BuildValue ("s", str);    // None if str==NULL
//    assert (str);   // or return None if string==NULL?
//    return PyString_FromString (str);
}


static PyObject *
py_csync_enable_conflictcopys (CSync *self)
{
    int rv = csync_enable_conflictcopys (self->ctx);
    assert (rv==0);
    Py_RETURN_NONE;
}


static PyObject *
py_csync_set_local_only (CSync *self, PyObject *args)
{
    int rv;
//    PyObject *tmp;

    // This will require an boolean
    // @todo does these set python exception if failes?
//    assert (PyTuple_Size (args) == 1);
//    //assert (PyBool_Check (PyTuple_GetItem (args, 1)));
//    tmp = PyTuple_GetItem (args, 1);
//    assert (PyBool_Check (tmp));
//    rv = csync_set_local_only (self->ctx, tmp==Py_True);
//    assert (rv==0);

    // This will accept any Python type and check if its true.
//    rv = PyObject_IsTrue (PyTuple_GetItem(args,0));
//    if (rv == -1) return NULL;

    // This will accept any(?) type convertible to integer.
    // Update: No, will accepts int (and bool is subtype of int)
    // @note Python3 supports "p" to accept a boolean predicate.
    if (! PyArg_ParseTuple (args, "i", &rv))
        return NULL;

    rv = csync_set_local_only (self->ctx, rv);
    assert (rv==0);

    Py_RETURN_NONE;
}


static PyObject *
py_csync_get_local_only (CSync *self)
{
//    if (csync_get_local_only (self->ctx))
//        Py_RETURN_TRUE;
//    else
//        Py_RETURN_FALSE;
    // Ok, since csync function can not fail.
    return PyBool_FromLong (csync_get_local_only (self->ctx));
}


// @note looks like visitor is called regardless of 'filter'
static PyObject *
py_csync_walk_local_tree (CSync *self, PyObject *args)
{
    return _py_csync_walk_tree (self, args, csync_walk_local_tree);
}


static PyObject *
py_csync_walk_remote_tree (CSync *self, PyObject *args)
{
    return _py_csync_walk_tree (self, args, csync_walk_remote_tree);
}


static PyObject *
py_csync_set_auth_callback (CSync *self, PyObject *args)
{
    PyObject *tmp;

    if (! PyArg_ParseTuple (args, "O", &tmp))
        return NULL;
    if (! PyCallable_Check (tmp)) {
        PyErr_SetString (PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

    int rv = csync_set_auth_callback (self->ctx, _py_auth_callback_wrapper);
    assert (rv == 0);

    Py_INCREF (tmp);    // ParseTuple returns borrowed ref, so must inc before storing in self
    Py_XDECREF (self->callbacks.auth);
    self->callbacks.auth = tmp;
    Py_RETURN_NONE;
}


// @todo return PyNone when NULL
static PyObject *
py_csync_get_auth_callback (CSync *self)
{
    Py_XINCREF (self->callbacks.auth);
    return self->callbacks.auth;
}


static PyObject *
py_csync_init (CSync *self)
{
    if (csync_init (self->ctx) == 0)
        Py_RETURN_NONE;

    return throw_csync_error (self);
}


static PyObject *
py_csync_add_exclude_list (CSync *self, PyObject *args)
{
    int rv;
    const char *path;

    if (! PyArg_ParseTuple (args, "s", &path))
        return NULL;

    // error if file does not exists. howto get error code / str?
//    rv = csync_add_exclude_list (self->ctx, "/tmp/a/xignore");
    rv = csync_add_exclude_list (self->ctx, path);
    assert (rv==0);

    Py_RETURN_NONE;
}


static PyObject *
py_csync_disable_statedb (CSync *self)
{
    int rv = csync_disable_statedb (self->ctx);
    assert (rv==0);
    Py_RETURN_NONE;
}

static PyObject *
py_csync_enable_statedb (CSync *self)
{
    int rv = csync_enable_statedb (self->ctx);
    assert (rv==0);
    Py_RETURN_NONE;
}


static PyObject *
py_csync_get_config_dir (CSync *self)
{
    const char *path = csync_get_config_dir (self->ctx);
    assert (path);
    return PyString_FromString (path);
}


// @note segfault if init() is not called first!
// @note segfault if statedb disabled (might return NULL? but howto report errors?)
static PyObject *
py_csync_get_statedb_file (CSync *self)
{
    const char *path = csync_get_statedb_file (self->ctx);
    assert (path);
    return PyString_FromString (path);
}


static PyObject *
py_csync_is_statedb_disabled (CSync *self)
{
    int enabled = csync_is_statedb_disabled (self->ctx);
    if (enabled)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
    // @todo PyBool_FromLong
}


static PyObject *
py_csync_propagate (CSync *self)
{
    int rv = csync_propagate (self->ctx);
    assert (rv==0);
    Py_RETURN_NONE;
}


static PyObject *
py_csync_reconcile (CSync *self)
{
    int rv = csync_reconcile (self->ctx);
    assert (rv==0);
    Py_RETURN_NONE;
}


static PyObject *
py_csync_update (CSync *self)
{
    int rv = csync_update (self->ctx);
    assert (rv==0);
    Py_RETURN_NONE;
}


static PyObject *
py_csync_commit (CSync *self)
{
    int rv = csync_commit (self->ctx);
    assert (rv==0);
    Py_RETURN_NONE;
}


static PyObject *
py_csync_set_config_dir (CSync *self, PyObject *args)
{
    int rv;
    const char *path;
    if (! PyArg_ParseTuple (args, "s", &path))
        return NULL;

    rv = csync_set_config_dir (self->ctx, path);
    assert (rv==0);
    Py_RETURN_NONE;
}



/*
// ImportError: build/lib.linux-x86_64-2.7/csync.so: undefined symbol:
// csync_remove_config_dir
static PyObject *
py_csync_remove_config_dir (CSync *self)
{
    int rv = csync_remove_config_dir (self->ctx);
    assert (rv==0);
    Py_RETURN_NONE;
}
*/


// XXX missing from my build
// has get_status() but doc says: Used for special modes or debugging
/*
static PyObject *
py_csync_status_ok (CSync *self)
{
    bool status = csync_status_ok (self->ctx);
    if (status)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}
*/



/**
 * CSync type definitions: Members, methods and the type object.
 */

// Members list
// CSync_members
static PyMemberDef CSyncMembers[] = {
//    {"first", T_OBJECT_EX, offsetof(Noddy, first), 0, "first name"},
//    {"number", T_INT, offsetof(Noddy, number), 0, "noddy number"},
    { NULL },
};


// Method list
// CSync_methods
static PyMethodDef CSyncMethods[] = {
    { "init",               (PyCFunction) py_csync_init,                METH_NOARGS,    "docstring" },
    { "add_exclude_list",   (PyCFunction) py_csync_add_exclude_list,    METH_VARARGS,   "docstring" },
    { "disable_statedb",    (PyCFunction) py_csync_disable_statedb,     METH_NOARGS,    "docstring" },
    { "enable_statedb",     (PyCFunction) py_csync_enable_statedb,      METH_NOARGS,    "docstring" },
    { "is_statedb_disabled",(PyCFunction) py_csync_is_statedb_disabled, METH_NOARGS,    "docstring" },
    { "get_statedb_file",   (PyCFunction) py_csync_get_statedb_file,    METH_NOARGS,    "docstring" },
    { "update",             (PyCFunction) py_csync_update,              METH_NOARGS,    "docstring" },
    { "propagate",          (PyCFunction) py_csync_propagate,           METH_NOARGS,    "docstring" },
    { "reconcile",          (PyCFunction) py_csync_reconcile,           METH_NOARGS,    "docstring" },
    { "commit",             (PyCFunction) py_csync_commit,              METH_NOARGS,    "docstring" },
    { "set_config_dir",     (PyCFunction) py_csync_set_config_dir,      METH_VARARGS,   "docstring" },
    { "get_config_dir",     (PyCFunction) py_csync_get_config_dir,      METH_NOARGS,    "docstring" },
//    { "remove_config_dir",  (PyCFunction) py_csync_remove_config_dir,   METH_NOARGS,    "docstring" },
    { "set_auth_callback",  (PyCFunction) py_csync_set_auth_callback,   METH_VARARGS,   "docstring" },
    { "get_auth_callback",  (PyCFunction) py_csync_get_auth_callback,   METH_NOARGS,    "docstring" },
    { "walk_local_tree",    (PyCFunction) py_csync_walk_local_tree,     METH_VARARGS,   "docstring" },
    { "walk_remote_tree",   (PyCFunction) py_csync_walk_remote_tree,    METH_VARARGS,   "docstring" },
    // @todo status_string member instead?
    { "get_status_string",  (PyCFunction) py_csync_get_status_string,   METH_NOARGS,    "docstring" },
#ifdef WITH_ICONV
    { "set_iconv_codec",    (PyCFunction) py_csync_set_iconv_codec,     METH_VARARGS,   "docstring" },
#endif
    { "enable_conflictcopys",   (PyCFunction) py_csync_enable_conflictcopys,    METH_NOARGS,    "docstring" },
    { "set_local_only",         (PyCFunction) py_csync_set_local_only,          METH_VARARGS,   "docstring" },
    { "get_local_only",         (PyCFunction) py_csync_get_local_only,          METH_NOARGS,    "docstring" },

    { "set_file_progress_callback", (PyCFunction) py_csync_set_file_progress_callback, METH_VARARGS, "docstring" },
    { "set_overall_progress_callback", (PyCFunction) py_csync_set_overall_progress_callback, METH_VARARGS, "docstring" },

    // just testing ...
    { "test", (PyCFunction) py_csync_test, METH_VARARGS, "docstring" },

    // @todo boolean statedb member?
//    { "status_ok", (PyCFunction) py_csync_status_ok, METH_NOARGS, "docstring" },
    { NULL },
};


// Type struct
static PyTypeObject CSyncType = {
    PyObject_HEAD_INIT(NULL)
    0,                              /* ob_size */
    "csync.CSync",                  /* tp_name */
    sizeof(CSync),                  /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor) _py_csync_dealloc, /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_compare */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,       /* tp_flags */
    "CSync objects",                /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    CSyncMethods,                   /* tp_methods */
    CSyncMembers,                   /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc) _py_csync_init,      /* tp_init */
    0,                              /* tp_alloc */
    _py_csync_new,                  /* tp_new */
};




/**
 * Module methods.
 */

// @note no argument => required version is same as build version
// @todo take required version in tuple? (not int)
static PyObject *
py_csync_version (PyObject *self, PyObject *args)
{
    int reqver = -1;
    if (! PyArg_ParseTuple (args, "|i", &reqver))
        return NULL;
    if (reqver == -1)
        reqver = CSYNC_VERSION_INT (LIBCSYNC_VERSION_MAJOR, LIBCSYNC_VERSION_MINOR, LIBCSYNC_VERSION_MICRO);

//    const char* verstr = csync_version (reqver);
//    if (verstr == NULL) Py_RETURN_NONE;
//    return PyString_FromString (NULL);

    return Py_BuildValue ("s", csync_version (reqver));  // NULL -> NoneType
}


static PyObject *
py_csync_set_log_level (PyObject *module, PyObject *args)
{
    int rv, level;
    if (! PyArg_ParseTuple (args, "i", &level))
        return NULL;

//    rv = csync_set_log_level ((int) PyInt_AsLong (tmp));
//    assert (rv==0);

    // @todo disallow -1 as loglevel? since csync_get_log_level
    // uses -1 to signal error.
//    level = (int) PyInt_AsLong (tmp);   // overflow possible?
//    printf ("%d\n", level);
//    if (level == -1) {
//        // check if error or value just happens to be -1
//        if (PyErr_Occurred())
//            assert (0);
//    }

    rv = csync_set_log_level (level);
    assert (rv==0);

    Py_RETURN_NONE;
}


static PyObject *
py_csync_get_log_level (PyObject *module)
{
    int level = csync_get_log_level ();
    assert (level!=-1);
    return PyInt_FromLong (level); // if error, NULL is returned
}


// @todo pass None to unset / reset to default
static PyObject *
py_csync_set_log_callback (PyObject *module, PyObject *args)
{
    PyObject *tmp;

    if (! PyArg_ParseTuple (args, "O", &tmp))
        return NULL;
    if (! PyCallable_Check (tmp)) {
        PyErr_SetString (PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

    int rv = csync_set_log_callback (_py_log_callback_wrapper);
    assert (rv == 0);

    Py_INCREF (tmp);
    Py_XDECREF (globals.log_callback);
    globals.log_callback = tmp;
    Py_RETURN_NONE;
}


static PyObject *
py_csync_get_log_callback (PyObject *module)
{
    if (! globals.log_callback)
        Py_RETURN_NONE;

    Py_INCREF (globals.log_callback);
    return globals.log_callback;
}



/** Module init. */

// Module methods
// @todo lower case?
static PyMethodDef CSyncModuleMethods[] = {
    { "version", py_csync_version, METH_VARARGS, "Get the csync version string." },
    { "set_log_level", py_csync_set_log_level, METH_VARARGS, "docstring" },
    { "get_log_level", (PyCFunction) py_csync_get_log_level, METH_NOARGS, "docstring" },
    { "set_log_callback", py_csync_set_log_callback, METH_VARARGS, "docstring" },
    { "get_log_callback", (PyCFunction) py_csync_get_log_callback, METH_NOARGS, "docstring" },
    { NULL }
};


// Only non-static function. Should be named "init<modulename>".
// @todo check return values
PyMODINIT_FUNC
initcsync (void)
{
    PyObject *mod;

    CSyncType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&CSyncType) < 0)
        return;
    
    mod = Py_InitModule3 ("csync", CSyncModuleMethods, "docstring");
    if (! mod) return;

    // Register type
    Py_INCREF (&CSyncType);
    PyModule_AddObject (mod, "CSync", (PyObject*) &CSyncType);

    /** Constants */
    PyModule_AddObject (mod, "VERSION", Py_BuildValue ("(iii)", LIBCSYNC_VERSION_MAJOR, LIBCSYNC_VERSION_MINOR, LIBCSYNC_VERSION_MICRO));
    //PyModule_AddStringConstant (mod, "VERSION_STRING", CSYNC_STRINGIFY(LIBCSYNC_VERSION));
    PyModule_AddIntConstant (mod, "VERSION_MAJOR", LIBCSYNC_VERSION_MAJOR);
    PyModule_AddIntConstant (mod, "VERSION_MINOR", LIBCSYNC_VERSION_MINOR);
    PyModule_AddIntConstant (mod, "VERSION_MICRO", LIBCSYNC_VERSION_MICRO);
    PyModule_AddStringConstant (mod, "CONF_DIR", CSYNC_CONF_DIR);
    PyModule_AddStringConstant (mod, "CONF_FILE", CSYNC_CONF_FILE);
    PyModule_AddStringConstant (mod, "EXCLUDE_FILE", CSYNC_EXCLUDE_FILE);
    PyModule_AddStringConstant (mod, "LOCK_FILE", CSYNC_LOCK_FILE);
#ifdef CSYNC_LOG_FILE
    PyModule_AddStringConstant (mod, "LOG_FILE", CSYNC_LOG_FILE);
#endif


    /** Error / exceptions */
    // @todo pull out into own function? (or file?)

    // Register enum constants. It might be more Pythonic to do it like
    // this: csync.STATUS_{ERROR,OK,etc}
    // But I like this better: csync.Status.{ERROR,OK,etc}
    PyObject *m = PyModule_New ("Status");
    size_t i;
    for (i=0; i<ARRAY_LENGTH(error_strings); ++i) {
        PyModule_AddIntConstant (m, error_strings[i].message,
                                 error_strings[i].code);
    }
    PyModule_AddObject (mod, "Status", m); // steals ref to m

    // Register custom exceptions
    // csync.Error members
    PyObject *dict, *key, *val;
    dict = PyDict_New();
    key = PyString_FromString ("status");
    val = PyInt_FromLong (CSYNC_STATUS_OK);
    PyDict_SetItem (dict, key, val);
    Py_DECREF (key);
    Py_DECREF (val);

//    CSyncError = PyErr_NewException ("csync.Error", NULL, NULL);
    CSyncError = PyErr_NewException ("csync.Error", NULL, dict);
    Py_INCREF (CSyncError);
    //Py_DECREF (dict);     // ???
    PyModule_AddObject (mod, "Error", CSyncError);
}
