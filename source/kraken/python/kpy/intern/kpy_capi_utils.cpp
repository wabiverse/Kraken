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

#include "kpy_capi_utils.h"

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
