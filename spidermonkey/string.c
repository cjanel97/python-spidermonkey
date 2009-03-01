#include "spidermonkey.h"

JSString*
py2js_string_obj(Context* cx, PyObject* str)
{
    JSString* rval = NULL;
    PyObject* conv = NULL;
    PyObject* encoded = NULL;
    char* bytes;
    Py_ssize_t len;

    if(PyString_Check(str))
    {
        conv = PyUnicode_FromEncodedObject(str, "utf-8", "replace");
        if(conv == NULL) return NULL;
        str = conv;
    }
    else if(!PyUnicode_Check(str))
    {
        PyErr_SetString(PyExc_TypeError, "Invalid string conversion.");
        return NULL;
    }

    encoded = PyUnicode_AsEncodedString(str, "utf-16", "strict");
    if(encoded == NULL) goto cleanup;
    if(PyString_AsStringAndSize(encoded, &bytes, &len) < 0) goto cleanup;
    if(len < 4) goto cleanup;

    rval = JS_NewUCStringCopyN(cx->cx, (jschar*) (bytes+2), (len/2)-1);
    
cleanup:
    Py_XDECREF(conv);
    return rval;
}

jsval
py2js_string(Context* cx, PyObject* str)
{
    JSString* val = py2js_string_obj(cx->cx, str);
    if(val == NULL)
    {
        PyErr_Clear();
        return JSVAL_VOID;
    }

    return STRING_TO_JSVAL(val);
}

PyObject*
js2py_string(Context* cx, jsval val)
{
    JSString* str;
    jschar* bytes;
    size_t len;

    if(!JSVAL_IS_STRING(val))
    {
        PyErr_SetString(PyExc_TypeError, "Value is not a JS String.");
        return NULL;
    }

    str = JSVAL_TO_STRING(val);
    len = JS_GetStringLength(str);
    bytes = JS_GetStringChars(str);

    return PyUnicode_Decode((const char*) bytes, len*2, "utf-16", "strict");
}