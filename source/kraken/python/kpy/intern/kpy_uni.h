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

#pragma once

/**
 * @file
 * KRAKEN Python.
 * It Bites.
 */

#include <Python.h>

#include "UNI_object.h"

#include "KKE_context.h"

#include "KPY_api.h"

/* --- bpy build options --- */
#ifdef WITH_PYTHON_SAFETY

/**
 * Play it safe and keep optional for now,
 * need to test further now this affects looping on 10000's of verts for eg.
 */
#  define USE_WEAKREFS

/* method to invalidate removed py data, XXX, slow to remove objects, otherwise no overhead */
/* #define USE_PYUNI_INVALIDATE_GC */

/* different method */
#  define USE_PYUNI_INVALIDATE_WEAKREF

/* support for inter references, currently only needed for corner case */
#  define USE_PYUNI_STRUCT_REFERENCE

#else /* WITH_PYTHON_SAFETY */

/* default, no defines! */

#endif /* !WITH_PYTHON_SAFETY */

WABI_NAMESPACE_BEGIN

extern PyTypeObject pyuni_struct_meta_idprop_Type;
extern PyTypeObject pyuni_struct_Type;
extern PyTypeObject pyuni_prop_Type;
extern PyTypeObject pyuni_prop_array_Type;
extern PyTypeObject pyuni_prop_collection_Type;
extern PyTypeObject pyuni_func_Type;

#define KPy_UniverseObject_Check(v) (PyObject_TypeCheck(v, &pyuni_struct_Type))
#define KPy_UniverseObject_CheckExact(v) (Py_TYPE(v) == &pyuni_struct_Type)
#define KPy_UniverseProperty_Check(v) (PyObject_TypeCheck(v, &pyuni_prop_Type))
#define KPy_UniverseProperty_CheckExact(v) (Py_TYPE(v) == &pyuni_prop_Type)

struct KPy_UniverseObject {
  PyObject_HEAD /* Required Python macro. */
#ifdef USE_WEAKREFS
  PyObject *in_weakreflist;
#endif
  PointerUNI ptr;
#ifdef USE_PYUNI_STRUCT_REFERENCE
  /**
   * generic PyObject we hold a reference to, example use:
   * hold onto the collection iterator to prevent it from
   * freeing allocated data we may use */
  PyObject *reference;
#endif /* !USE_PYUNI_STRUCT_REFERENCE */

#ifdef PYUNI_FREE_SUPPORT
  bool freeptr; /* needed in some cases if ptr.data is created on the fly, free when deallocing */
#endif          /* PYUNI_FREE_SUPPORT */
};

/* kpy.utils.(un)register_class */
extern PyMethodDef meth_kpy_register_class;
extern PyMethodDef meth_kpy_unregister_class;

/* kpy.utils._kr_owner_(get/set) */
extern PyMethodDef meth_kpy_owner_id_set;
extern PyMethodDef meth_kpy_owner_id_get;

extern KPy_UniverseObject *kpy_context_module;

WABI_NAMESPACE_END