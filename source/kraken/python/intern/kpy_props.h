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

#pragma once

/**
 * @file
 * KRAKEN Python.
 * It Bites.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define PRIM_STACK_ARRAY 32

PyObject *KPY_prim_props(void);
/**
 * Run this on exit, clearing all Python callback users and disable the RNA callback,
 * as it would be called after Python has already finished.
 */
void KPY_prim_props_clear_all(void);

PyObject *KPy_PointerProperty(PyObject *self, PyObject *args, PyObject *kw);
PyObject *KPy_CollectionProperty(PyObject *self, PyObject *args, PyObject *kw);
KrakenPRIM *pointer_type_from_py(PyObject *value, const char *error_prefix);

typedef struct
{
  PyObject_HEAD
    /**
     * Internally a #PyCFunctionObject type.
     * @note This isn't GC tracked, it's a function from `kpy.props` so it's not going away.
     */
    PyObject *fn;
  PyObject *kw;
} KPy_PropDeferred;

extern PyTypeObject kpy_prop_deferred_Type;
#define KPy_PropDeferred_CheckTypeExact(v) (Py_TYPE(v) == &kpy_prop_deferred_Type)

#define PYPRIM_STACK_ARRAY PRIM_STACK_ARRAY

#ifdef __cplusplus
}
#endif
