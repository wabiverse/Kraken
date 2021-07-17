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
 * Copyright 2021, Wabi.
 */

/**
 * @file
 * KRAKEN Python.
 * It Bites.
 */

#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <frameobject.h>

#include "KLI_string_utils.h"

#include "kpy_capi_utils.h"

#include <wabi/base/arch/hints.h>

WABI_NAMESPACE_BEGIN

/**
 * Use with PyArg_ParseTuple's "O&" formatting. */
int PyC_ParseStringEnum(PyObject *o, void *p)
{
  struct PyC_StringEnum *e = (PyC_StringEnum *)p;
  const char *value = PyUnicode_AsUTF8(o);
  if (value == NULL)
  {
    PyErr_Format(PyExc_ValueError, "expected a string, got %s", Py_TYPE(o)->tp_name);
    return 0;
  }
  int i;
  for (i = 0; e->items[i].id; i++)
  {
    if (STREQ(e->items[i].id, value))
    {
      e->value_found = e->items[i].value;
      return 1;
    }
  }

  /* Set as a precaution. */
  e->value_found = -1;

  PyObject *enum_items = PyTuple_New(i);
  for (i = 0; e->items[i].id; i++)
  {
    PyTuple_SET_ITEM(enum_items, i, PyUnicode_FromString(e->items[i].id));
  }
  PyErr_Format(PyExc_ValueError, "expected a string in %S, got '%s'", enum_items, value);
  Py_DECREF(enum_items);
  return 0;
}


/**
 * Use with PyArg_ParseTuple's "O&" formatting.
 *
 * @see #PyC_Long_AsBool for a similar function to use outside of argument parsing. */
int PyC_ParseBool(PyObject *o, void *p)
{
  bool *bool_p = (bool *)p;
  long value;
  if (((value = PyLong_AsLong(o)) == -1) || ((value != 0) && (value != 1)))
  {
    PyErr_Format(PyExc_ValueError, "expected a bool or int (0/1), got %s", Py_TYPE(o)->tp_name);
    return 0;
  }

  *bool_p = value ? true : false;
  return 1;
}

PyObject *PyC_UnicodeFromByteAndSize(const char *str, Py_ssize_t size)
{
  PyObject *result = PyUnicode_FromStringAndSize(str, size);
  if (result)
  {
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
  if (ARCH_UNLIKELY((uint)test > 1))
  {
    PyErr_SetString(PyExc_TypeError, "Python number not a bool (0/1)");
    return -1;
  }
  return test;
}

int8_t PyC_Long_AsI8(PyObject *value)
{
  const int test = _PyLong_AsInt(value);
  if (ARCH_UNLIKELY(test < INT8_MIN || test > INT8_MAX))
  {
    PyErr_SetString(PyExc_OverflowError, "Python int too large to convert to C int8");
    return -1;
  }
  return (int8_t)test;
}

int16_t PyC_Long_AsI16(PyObject *value)
{
  const int test = _PyLong_AsInt(value);
  if (ARCH_UNLIKELY(test < INT16_MIN || test > INT16_MAX))
  {
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
  if (ARCH_UNLIKELY(test > UINT8_MAX))
  {
    PyErr_SetString(PyExc_OverflowError, "Python int too large to convert to C uint8");
    return (uint8_t)-1;
  }
  return (uint8_t)test;
}

uint16_t PyC_Long_AsU16(PyObject *value)
{
  const ulong test = PyLong_AsUnsignedLong(value);
  if (ARCH_UNLIKELY(test > UINT16_MAX))
  {
    PyErr_SetString(PyExc_OverflowError, "Python int too large to convert to C uint16");
    return (uint16_t)-1;
  }
  return (uint16_t)test;
}

uint32_t PyC_Long_AsU32(PyObject *value)
{
  const ulong test = PyLong_AsUnsignedLong(value);
  if (ARCH_UNLIKELY(test > UINT32_MAX))
  {
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
/** \name Exception Utilities
 * \{ */

/**
 * Similar to #PyErr_Format(),
 *
 * Implementation - we can't actually prepend the existing exception,
 * because it could have _any_ arguments given to it, so instead we get its
 * ``__str__`` output and raise our own exception including it.
 */
PyObject *PyC_Err_Format_Prefix(PyObject *exception_type_prefix, const char *format, ...)
{
  PyObject *error_value_prefix;
  va_list args;

  va_start(args, format);
  error_value_prefix = PyUnicode_FromFormatV(format, args); /* can fail and be NULL */
  va_end(args);

  if (PyErr_Occurred())
  {
    PyObject *error_type, *error_value, *error_traceback;
    PyErr_Fetch(&error_type, &error_value, &error_traceback);

    if (PyUnicode_Check(error_value))
    {
      PyErr_Format(exception_type_prefix, "%S, %S", error_value_prefix, error_value);
    }
    else
    {
      PyErr_Format(exception_type_prefix,
                   "%S, %.200s(%S)",
                   error_value_prefix,
                   Py_TYPE(error_value)->tp_name,
                   error_value);
    }
  }
  else
  {
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

/** \} */

WABI_NAMESPACE_END