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

WABI_NAMESPACE_BEGIN

/**
 * Use with PyArg_ParseTuple's "O&" formatting. */
int PyC_ParseStringEnum(PyObject *o, void *p)
{
  struct PyC_StringEnum *e = (PyC_StringEnum*)p;
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

WABI_NAMESPACE_END