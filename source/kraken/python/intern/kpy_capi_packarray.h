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

#ifndef __PY_CAPI_PACKARRAY_H__
#define __PY_CAPI_PACKARRAY_H__

/**
 * @file
 * KRAKEN Python.
 * It Bites.
 */

#include <Python.h>

#include "KLI_math.h"

#include <stdbool.h>

/**
 * Not conformant with modern CXX code. Anonymous Array's are
 * from pre-C++0x -- offloading these functions to compile in
 * C land. */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

PyObject *PyC_Tuple_PackArray_F32(const float *array, uint len);
PyObject *PyC_Tuple_PackArray_F64(const double *array, uint len);
PyObject *PyC_Tuple_PackArray_I32(const int *array, uint len);
PyObject *PyC_Tuple_PackArray_I32FromBool(const int *array, uint len);
PyObject *PyC_Tuple_PackArray_Bool(const bool *array, uint len);

#define PyC_Tuple_Pack_F32(...) \
  PyC_Tuple_PackArray_F32(((const float[]){__VA_ARGS__}), VA_NARGS_COUNT(__VA_ARGS__))
#define PyC_Tuple_Pack_F64(...) \
  PyC_Tuple_PackArray_F64(((const double[]){__VA_ARGS__}), VA_NARGS_COUNT(__VA_ARGS__))
#define PyC_Tuple_Pack_I32(...) \
  PyC_Tuple_PackArray_I32(((const int[]){__VA_ARGS__}), VA_NARGS_COUNT(__VA_ARGS__))
#define PyC_Tuple_Pack_I32FromBool(...) \
  PyC_Tuple_PackArray_I32FromBool(((const int[]){__VA_ARGS__}), VA_NARGS_COUNT(__VA_ARGS__))
#define PyC_Tuple_Pack_Bool(...) \
  PyC_Tuple_PackArray_Bool(((const bool[]){__VA_ARGS__}), VA_NARGS_COUNT(__VA_ARGS__))

void SetObjItemIncrementInfo(PyObject *ob, int pos);
void SetObjItemBoolIncrementInfo(PyObject *ob, int pos, bool val);

#ifdef __cplusplus
} /* extern C */
#endif /* __cplusplus */


#endif /* __PY_CAPI_PACKARRAY_H__ */
