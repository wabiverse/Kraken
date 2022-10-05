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

#include "kraken/kraken.h"

#include "../generic/py_capi_utils.h"
#include "../generic/python_utildefines.h"

#include "kpy_capi_packarray.h"

/* -------------------------------------------------------------------- */
/** \name Typed Tuple Packing
 *
 * @note See #PyC_Tuple_Pack_* macros that take multiple arguments.
 * \{ */

/* array utility function */
PyObject *PyC_Tuple_PackArray_F32(const float *array, uint len)
{
  PyObject *tuple = PyTuple_New(len);
  for (uint i = 0; i < len; i++)
  {
    PyTuple_SET_ITEM(tuple, i, PyFloat_FromDouble(array[i]));
  }
  return tuple;
}

PyObject *PyC_Tuple_PackArray_F64(const double *array, uint len)
{
  PyObject *tuple = PyTuple_New(len);
  for (uint i = 0; i < len; i++)
  {
    PyTuple_SET_ITEM(tuple, i, PyFloat_FromDouble(array[i]));
  }
  return tuple;
}

PyObject *PyC_Tuple_PackArray_I32(const int *array, uint len)
{
  PyObject *tuple = PyTuple_New(len);
  for (uint i = 0; i < len; i++)
  {
    PyTuple_SET_ITEM(tuple, i, PyLong_FromLong(array[i]));
  }
  return tuple;
}

PyObject *PyC_Tuple_PackArray_I32FromBool(const int *array, uint len)
{
  PyObject *tuple = PyTuple_New(len);
  for (uint i = 0; i < len; i++)
  {
    PyTuple_SET_ITEM(tuple, i, PyBool_FromLong(array[i]));
  }
  return tuple;
}

PyObject *PyC_Tuple_PackArray_Bool(const bool *array, uint len)
{
  PyObject *tuple = PyTuple_New(len);
  for (uint i = 0; i < len; i++)
  {
    PyTuple_SET_ITEM(tuple, i, PyBool_FromLong(array[i]));
  }
  return tuple;
}

void SetObjItemIncrementInfo(PyObject *app_info, int pos)
{
#define SetObjItem(obj) PyStructSequence_SET_ITEM(app_info, pos++, obj)

  SetObjItem(PyC_Tuple_Pack_I32(KRAKEN_VERSION / 100, KRAKEN_VERSION % 100, KRAKEN_VERSION_PATCH));
  SetObjItem(
    PyC_Tuple_Pack_I32(KRAKEN_FILE_VERSION / 100, KRAKEN_FILE_VERSION % 100, KRAKEN_FILE_SUBVERSION));
}

void SetObjItemBoolIncrementInfo(PyObject *app_info, int pos, bool val)
{
#define SetObjItem(obj) PyStructSequence_SET_ITEM(app_info, pos++, obj)

  SetObjItem(PyBool_FromLong(val));
}

/** \} */