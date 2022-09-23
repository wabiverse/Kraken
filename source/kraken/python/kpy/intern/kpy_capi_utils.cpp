/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * KRAKEN Python.
 * It Bites.
 */

#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <frameobject.h>

#include "KLI_string.h"

#include "KKE_report.h"

#include "kpy_capi_utils.h"

#include <wabi/base/arch/hints.h>

#include <boost/python.hpp>
#include <boost/python/overloads.hpp>

using namespace boost::python;

KRAKEN_NAMESPACE_USING

/**
 * Use with PyArg_ParseTuple's "O&" formatting. */
int PyC_ParseStringEnum(PyObject *o, void *p)
{
  struct PyC_StringEnum *e = (PyC_StringEnum *)p;
  const char *value = PyUnicode_AsUTF8(o);
  if (value == NULL) {
    PyErr_Format(PyExc_ValueError, "expected a string, got %s", Py_TYPE(o)->tp_name);
    return 0;
  }
  int i;
  for (i = 0; e->items[i].id; i++) {
    if (STREQ(e->items[i].id, value)) {
      e->value_found = e->items[i].value;
      return 1;
    }
  }

  /* Set as a precaution. */
  e->value_found = -1;

  PyObject *enum_items = PyTuple_New(i);
  for (i = 0; e->items[i].id; i++) {
    PyTuple_SET_ITEM(enum_items, i, PyUnicode_FromString(e->items[i].id));
  }
  PyErr_Format(PyExc_ValueError, "expected a string in %S, got '%s'", enum_items, value);
  Py_DECREF(enum_items);
  return 0;
}

void PyC_ObSpit(const char *name, PyObject *var)
{
  const char *null_str = "<null>";
  fprintf(stderr, "<%s> : ", name);
  if (var == NULL) {
    fprintf(stderr, "%s\n", null_str);
  } else {
    PyObject_Print(var, stderr, 0);
    const PyTypeObject *type = Py_TYPE(var);
    fprintf(stderr,
            " ref:%d, ptr:%p, type: %s\n",
            (int)var->ob_refcnt,
            (void *)var,
            type ? type->tp_name : null_str);
  }
}

void PyC_ObSpitStr(char *result, size_t result_len, PyObject *var)
{
  /* No name, creator of string can manage that. */
  const char *null_str = "<null>";
  if (var == NULL) {
    KLI_snprintf(result, result_len, "%s", null_str);
  } else {
    const PyTypeObject *type = Py_TYPE(var);
    PyObject *var_str = PyObject_Repr(var);
    if (var_str == NULL) {
      /* We could print error here,
       * but this may be used for generating errors - so don't for now. */
      PyErr_Clear();
    }
    KLI_snprintf(result,
                 result_len,
                 " ref=%d, ptr=%p, type=%s, value=%.200s",
                 (int)var->ob_refcnt,
                 (void *)var,
                 type ? type->tp_name : null_str,
                 var_str ? PyUnicode_AsUTF8(var_str) : "<error>");
    if (var_str != NULL) {
      Py_DECREF(var_str);
    }
  }
}


/**
 * Use with PyArg_ParseTuple's "O&" formatting.
 *
 * @see #PyC_Long_AsBool for a similar function to use outside of argument parsing. */
int PyC_ParseBool(PyObject *o, void *p)
{
  bool *bool_p = (bool *)p;
  long value;
  if (((value = PyLong_AsLong(o)) == -1) || ((value != 0) && (value != 1))) {
    PyErr_Format(PyExc_ValueError, "expected a bool or int (0/1), got %s", Py_TYPE(o)->tp_name);
    return 0;
  }

  *bool_p = value ? true : false;
  return 1;
}

PyObject *PyC_UnicodeFromByteAndSize(const char *str, Py_ssize_t size)
{
  PyObject *result = PyUnicode_FromStringAndSize(str, size);
  if (result) {
    /* 99% of the time this is enough but we better support non unicode
     * chars since blender doesn't limit this */
    return result;
  }

  PyErr_Clear();
  /* this means paths will always be accessible once converted, on all OS's */
  result = PyUnicode_DecodeFSDefaultAndSize(str, size);
  return result;
}

PyObject *PyC_UnicodeFromByte(const char *str)
{
  return PyC_UnicodeFromByteAndSize(str, strlen(str));
}

/* -------------------------------------------------------------------- */
/** \name Int Conversion
 *
 * \note Python doesn't provide overflow checks for specific bit-widths.
 *
 * \{ */

/* Compiler optimizes out redundant checks. */
#ifdef __GNUC__
#  pragma warning(push)
#  pragma GCC diagnostic ignored "-Wtype-limits"
#endif

/**
 * Don't use `bool` return type, so -1 can be used as an error value.
 */
int PyC_Long_AsBool(PyObject *value)
{
  const int test = _PyLong_AsInt(value);
  if (ARCH_UNLIKELY((uint)test > 1)) {
    PyErr_SetString(PyExc_TypeError, "Python number not a bool (0/1)");
    return -1;
  }
  return test;
}

int8_t PyC_Long_AsI8(PyObject *value)
{
  const int test = _PyLong_AsInt(value);
  if (ARCH_UNLIKELY(test < INT8_MIN || test > INT8_MAX)) {
    PyErr_SetString(PyExc_OverflowError, "Python int too large to convert to C int8");
    return -1;
  }
  return (int8_t)test;
}

int16_t PyC_Long_AsI16(PyObject *value)
{
  const int test = _PyLong_AsInt(value);
  if (ARCH_UNLIKELY(test < INT16_MIN || test > INT16_MAX)) {
    PyErr_SetString(PyExc_OverflowError, "Python int too large to convert to C int16");
    return -1;
  }
  return (int16_t)test;
}

/* Inlined in header:
 * PyC_Long_AsI32
 * PyC_Long_AsI64
 */

uint8_t PyC_Long_AsU8(PyObject *value)
{
  const ulong test = PyLong_AsUnsignedLong(value);
  if (ARCH_UNLIKELY(test > UINT8_MAX)) {
    PyErr_SetString(PyExc_OverflowError, "Python int too large to convert to C uint8");
    return (uint8_t)-1;
  }
  return (uint8_t)test;
}

uint16_t PyC_Long_AsU16(PyObject *value)
{
  const ulong test = PyLong_AsUnsignedLong(value);
  if (ARCH_UNLIKELY(test > UINT16_MAX)) {
    PyErr_SetString(PyExc_OverflowError, "Python int too large to convert to C uint16");
    return (uint16_t)-1;
  }
  return (uint16_t)test;
}

uint32_t PyC_Long_AsU32(PyObject *value)
{
  const ulong test = PyLong_AsUnsignedLong(value);
  if (ARCH_UNLIKELY(test > UINT32_MAX)) {
    PyErr_SetString(PyExc_OverflowError, "Python int too large to convert to C uint32");
    return (uint32_t)-1;
  }
  return (uint32_t)test;
}

/* Inlined in header:
 * PyC_Long_AsU64
 */

#ifdef __GNUC__
#  pragma warning(pop)
#endif

/** \} */

/* -------------------------------------------------------------------- */
/** \name Access Current Frame File Name & Line Number
 * \{ */

void PyC_FileAndNum(const char **r_filename, int *r_lineno)
{
  PyFrameObject *frame;

  if (r_filename) {
    *r_filename = NULL;
  }
  if (r_lineno) {
    *r_lineno = -1;
  }

  if (!(frame = PyThreadState_GET()->frame)) {
    return;
  }

  /* when executing a script */
  if (r_filename) {
    *r_filename = PyUnicode_AsUTF8(frame->f_code->co_filename);
  }

  /* when executing a module */
  if (r_filename && *r_filename == NULL) {
    /* try an alternative method to get the r_filename - module based
     * references below are all borrowed (double checked) */
    PyObject *mod_name = PyDict_GetItemString(PyEval_GetGlobals(), "__name__");
    if (mod_name) {
      PyObject *mod = PyDict_GetItem(PyImport_GetModuleDict(), mod_name);
      if (mod) {
        PyObject *mod_file = PyModule_GetFilenameObject(mod);
        if (mod_file) {
          *r_filename = PyUnicode_AsUTF8(mod_name);
          Py_DECREF(mod_file);
        } else {
          PyErr_Clear();
        }
      }

      /* unlikely, fallback */
      if (*r_filename == NULL) {
        *r_filename = PyUnicode_AsUTF8(mod_name);
      }
    }
  }

  if (r_lineno) {
    *r_lineno = PyFrame_GetLineNumber(frame);
  }
}

void PyC_FileAndNum_Safe(const char **r_filename, int *r_lineno)
{
  if (!PyC_IsInterpreterActive()) {
    return;
  }

  PyC_FileAndNum(r_filename, r_lineno);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Frozen Set Creation
 * \{ */

PyObject *PyC_FrozenSetFromStrings(const char **strings)
{
  const char **str;
  PyObject *ret;

  ret = PyFrozenSet_New(NULL);

  for (str = strings; *str; str++) {
    PyObject *py_str = PyUnicode_FromString(*str);
    PySet_Add(ret, py_str);
    Py_DECREF(py_str);
  }

  return ret;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Exception Utilities
 * \{ */

/**
 * Similar to #PyErr_Format(),
 *
 * Implementation - we can't actually prepend the existing exception,
 * because it could have _any_ arguments given to it, so instead we get its
 * ``__str__`` output and raise our own exception including it. */
PyObject *PyC_Err_Format_Prefix(PyObject *exception_type_prefix, const char *format, ...)
{
  PyObject *error_value_prefix;
  va_list args;

  va_start(args, format);
  error_value_prefix = PyUnicode_FromFormatV(format, args); /* can fail and be NULL */
  va_end(args);

  if (PyErr_Occurred()) {
    PyObject *error_type, *error_value, *error_traceback;
    PyErr_Fetch(&error_type, &error_value, &error_traceback);

    if (PyUnicode_Check(error_value)) {
      PyErr_Format(exception_type_prefix, "%S, %S", error_value_prefix, error_value);
    } else {
      PyErr_Format(exception_type_prefix,
                   "%S, %.200s(%S)",
                   error_value_prefix,
                   Py_TYPE(error_value)->tp_name,
                   error_value);
    }
  } else {
    PyErr_SetObject(exception_type_prefix, error_value_prefix);
  }

  Py_XDECREF(error_value_prefix);

  /* dumb to always return NULL but matches PyErr_Format */
  return NULL;
}

PyObject *PyC_Err_SetString_Prefix(PyObject *exception_type_prefix, const char *str)
{
  return PyC_Err_Format_Prefix(exception_type_prefix, "%s", str);
}

PyObject *PyC_ExceptionBuffer(void)
{
  PyObject *stdout_backup = PySys_GetObject("stdout"); /* borrowed */
  PyObject *stderr_backup = PySys_GetObject("stderr"); /* borrowed */
  PyObject *string_io = NULL;
  PyObject *string_io_buf = NULL;
  PyObject *string_io_mod = NULL;
  PyObject *string_io_getvalue = NULL;

  PyObject *error_type, *error_value, *error_traceback;

  if (!PyErr_Occurred()) {
    return NULL;
  }

  PyErr_Fetch(&error_type, &error_value, &error_traceback);

  PyErr_Clear();

  /* import io
   * string_io = io.StringIO()
   */

  if (!(string_io_mod = PyImport_ImportModule("io"))) {
    goto error_cleanup;
  } else if (!(string_io = PyObject_CallMethod(string_io_mod, "StringIO", NULL))) {
    goto error_cleanup;
  } else if (!(string_io_getvalue = PyObject_GetAttrString(string_io, "getvalue"))) {
    goto error_cleanup;
  }

  /* Since these were borrowed we don't want them freed when replaced. */
  Py_INCREF(stdout_backup);
  Py_INCREF(stderr_backup);

  /* Both of these are freed when restoring. */
  PySys_SetObject("stdout", string_io);
  PySys_SetObject("stderr", string_io);

  PyErr_Restore(error_type, error_value, error_traceback);
  PyErr_Print(); /* print the error */
  PyErr_Clear();

  string_io_buf = PyObject_CallObject(string_io_getvalue, NULL);

  PySys_SetObject("stdout", stdout_backup);
  PySys_SetObject("stderr", stderr_backup);

  Py_DECREF(stdout_backup); /* now sys owns the ref again */
  Py_DECREF(stderr_backup);

  Py_DECREF(string_io_mod);
  Py_DECREF(string_io_getvalue);
  Py_DECREF(string_io); /* free the original reference */

  PyErr_Clear();
  return string_io_buf;

error_cleanup:
  /* could not import the module so print the error and close */
  Py_XDECREF(string_io_mod);
  Py_XDECREF(string_io);

  PyErr_Restore(error_type, error_value, error_traceback);
  PyErr_Print(); /* print the error */
  PyErr_Clear();

  return NULL;
}

PyObject *PyC_ExceptionBuffer_Simple(void)
{
  PyObject *string_io_buf = NULL;

  PyObject *error_type, *error_value, *error_traceback;

  if (!PyErr_Occurred()) {
    return NULL;
  }

  PyErr_Fetch(&error_type, &error_value, &error_traceback);

  if (error_value == NULL) {
    return NULL;
  }

  if (PyErr_GivenExceptionMatches(error_type, PyExc_SyntaxError)) {
    /* Special exception for syntax errors,
     * in these cases the full error is verbose and not very useful,
     * just use the initial text so we know what the error is. */
    if (PyTuple_CheckExact(error_value) && PyTuple_GET_SIZE(error_value) >= 1) {
      string_io_buf = PyObject_Str(PyTuple_GET_ITEM(error_value, 0));
    }
  }

  if (string_io_buf == NULL) {
    string_io_buf = PyObject_Str(error_value);
  }

  /* Python does this too */
  if (ARCH_UNLIKELY(string_io_buf == NULL)) {
    string_io_buf = PyUnicode_FromFormat("<unprintable %s object>", Py_TYPE(error_value)->tp_name);
  }

  PyErr_Restore(error_type, error_value, error_traceback);

  PyErr_Clear();
  return string_io_buf;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Name Space Creation/Manipulation
 * \{ */

PyObject *PyC_DefaultNameSpace(const char *filename)
{
  PyObject *modules = PyImport_GetModuleDict();
  PyObject *builtins = PyEval_GetBuiltins();
  PyObject *mod_main = PyModule_New("__main__");
  PyDict_SetItemString(modules, "__main__", mod_main);
  Py_DECREF(mod_main); /* sys.modules owns now */
  PyModule_AddStringConstant(mod_main, "__name__", "__main__");
  if (filename) {
    /* __file__ mainly for nice UI'ness
     * note: this won't map to a real file when executing text-blocks and buttons. */
    PyModule_AddObject(mod_main, "__file__", PyC_UnicodeFromByte(filename));
  }
  PyModule_AddObject(mod_main, "__builtins__", builtins);
  Py_INCREF(builtins); /* AddObject steals a reference */
  return PyModule_GetDict(mod_main);
}

bool PyC_NameSpace_ImportArray(PyObject *py_dict, const char *imports[])
{
  for (int i = 0; imports[i]; i++) {
    PyObject *name = PyUnicode_FromString(imports[i]);
    PyObject *mod = PyImport_ImportModuleLevelObject(name, NULL, NULL, 0, 0);
    bool ok = false;
    if (mod) {
      PyDict_SetItem(py_dict, name, mod);
      ok = true;
      Py_DECREF(mod);
    }
    Py_DECREF(name);

    if (!ok) {
      return false;
    }
  }
  return true;
}

/* restore MUST be called after this */
void PyC_MainModule_Backup(PyObject **r_main_mod)
{
  PyObject *modules = PyImport_GetModuleDict();
  *r_main_mod = PyDict_GetItemString(modules, "__main__");
  Py_XINCREF(*r_main_mod); /* don't free */
}

void PyC_MainModule_Restore(PyObject *main_mod)
{
  PyObject *modules = PyImport_GetModuleDict();
  PyDict_SetItemString(modules, "__main__", main_mod);
  Py_XDECREF(main_mod);
}

bool PyC_IsInterpreterActive(void)
{
  /* instead of PyThreadState_Get, which calls Py_FatalError */
  return (PyThreadState_GetDict() != NULL);
}

/** \} */

bool KPy_errors_to_report_ex(kraken::ReportList *reports,
                             const char *error_prefix,
                             const bool use_full,
                             const bool use_location)
{
  PyObject *pystring;

  if (!PyErr_Occurred()) {
    return 1;
  }

  /* less hassle if we allow NULL */
  if (reports == NULL) {
    PyErr_Print();
    PyErr_Clear();
    return 1;
  }

  if (use_full) {
    pystring = PyC_ExceptionBuffer();
  } else {
    pystring = PyC_ExceptionBuffer_Simple();
  }

  if (pystring == NULL) {
    KKE_report(reports, RPT_ERROR, "Unknown py-exception, could not convert");
    return 0;
  }

  if (error_prefix == NULL) {
    /* Not very helpful, better than nothing. */
    error_prefix = "Python";
  }

  if (use_location) {
    const char *filename;
    int lineno;

    PyC_FileAndNum(&filename, &lineno);
    if (filename == NULL) {
      filename = "<unknown location>";
    }

    KKE_reportf(reports,
                RPT_ERROR,
                TIP_("%s: %s\nlocation: %s:%d\n"),
                error_prefix,
                PyUnicode_AsUTF8(pystring),
                filename,
                lineno);

    /* Not exactly needed. Useful for developers tracking down issues. */
    fprintf(stderr,
            TIP_("%s: %s\nlocation: %s:%d\n"),
            error_prefix,
            PyUnicode_AsUTF8(pystring),
            filename,
            lineno);
  } else {
    KKE_reportf(reports, RPT_ERROR, "%s: %s", error_prefix, PyUnicode_AsUTF8(pystring));
  }

  Py_DECREF(pystring);
  return 1;
}

short KPy_reports_to_error(kraken::ReportList *reports, PyObject *exception, const bool clear)
{
  char *report_str = nullptr;

  report_str = KKE_reports_string(reports, RPT_ERROR);

  if (clear == true) {
    KKE_reports_clear(reports);
  }

  if (report_str) {
    PyErr_SetString(exception, report_str);
    free(report_str);
  }

  return (report_str == NULL) ? 0 : -1;
}

bool KPy_errors_to_report(kraken::ReportList *reports)
{
  return KPy_errors_to_report_ex(reports, NULL, true, true);
}

void KPy_reports_write_stdout(const kraken::ReportList *reports, const char *header)
{
  if (header) {
    PySys_WriteStdout("%s\n", header);
  }

  for (auto &report : reports->list) {
    PySys_WriteStdout("%s: %s\n", report->typestr, report->message);
  }
}
