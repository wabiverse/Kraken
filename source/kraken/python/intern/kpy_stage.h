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

#include <Python.h>

#include "USD_object.h"

#include "KKE_context.h"

#include "KPY_api.h"

/* --- kpy build options --- */
#ifdef WITH_PYTHON_SAFETY

/**
 * Play it safe and keep optional for now,
 * need to test further now this affects looping on 10000's of verts for eg.
 */
#  define USE_WEAKREFS

/* method to invalidate removed py data, XXX, slow to remove objects, otherwise no overhead */
/* #define USE_PYUSD_INVALIDATE_GC */

/* different method */
#  define USE_PYUSD_INVALIDATE_WEAKREF

/* support for inter references, currently only needed for corner case */
#  define USE_PYUSD_OBJECT_REFERENCE

#else /* WITH_PYTHON_SAFETY */

/* default, no defines! */

#endif /* !WITH_PYTHON_SAFETY */

#define USE_PYUSD_ITER

// struct KPy_DummyKrakenPRIM
// {
//   PyObject_HEAD /* Required Python macro. */
// #ifdef USE_WEAKREFS
//     PyObject *in_weakreflist;
// #endif
//   KrakenPRIM ptr;
// };

struct KPy_KrakenStage
{
  PyObject_HEAD /* Required Python macro. */
#ifdef USE_WEAKREFS
    PyObject *in_weakreflist;
#endif
  wabi::UsdStageWeakPtr ptr;
  void *data;
#ifdef USE_PYUSD_OBJECT_REFERENCE
  /**
   * generic PyObject we hold a reference to, example use:
   * hold onto the collection iterator to prevent it from
   * freeing allocated data we may use */
  PyObject *reference;
#endif /* !USE_PYUSD_OBJECT_REFERENCE */

#ifdef PYUSD_FREE_SUPPORT
  bool freeptr; /* needed in some cases if ptr.data is created on the fly, free when deallocing */
#endif          /* PYUSD_FREE_SUPPORT */
};

struct KPy_KrakenPROP
{
  PyObject_HEAD /* Required Python macro. */
#ifdef USE_WEAKREFS
    PyObject *in_weakreflist;
#endif
  KrakenPRIM ptr;
  // KrakenPROP *prop;
};

struct KPy_KrakenFUNC
{
  PyObject_HEAD /* Required Python macro. */
#ifdef USE_WEAKREFS
    PyObject *in_weakreflist;
#endif
  KrakenPRIM ptr;
  KrakenFUNC *func;
};

struct KPy_UsdPropertyVector
{
  PyObject_HEAD /* Required Python macro. */
#ifdef USE_WEAKREFS
    PyObject *in_weakreflist;
#endif

  /* collection iterator specific parts */
  ListBase iter;
};

#ifdef __cplusplus
extern "C" {
#endif

extern PyTypeObject pystage_struct_meta_idprop_Type;
extern PyTypeObject pystage_struct_Type;
// extern PyTypeObject pystage_prop_Type;
// extern PyTypeObject pystage_prop_array_Type;
// extern PyTypeObject pystage_prop_collection_Type;
// extern PyTypeObject pystage_func_Type;

#define KPy_KrakenStage_Check(v) (PyObject_TypeCheck(v, &pystage_struct_Type))
#define KPy_KrakenStage_CheckExact(v) (Py_TYPE(v) == &pystage_struct_Type)
// #define KPy_KrakenPROP_Check(v) (PyObject_TypeCheck(v, &pystage_prop_Type))
// #define KPy_KrakenPROP_CheckExact(v) (Py_TYPE(v) == &pystage_prop_Type)

#define PYUSD_STRUCT_CHECK_OBJ(obj)                              \
  if (ARCH_UNLIKELY(pystage_struct_validity_check(obj) == -1)) { \
    return NULL;                                                 \
  }                                                              \
  (void)0
#define PYUSD_STRUCT_CHECK_INT(obj)                              \
  if (ARCH_UNLIKELY(pystage_struct_validity_check(obj) == -1)) { \
    return -1;                                                   \
  }                                                              \
  (void)0

#define PYUSD_PROP_CHECK_OBJ(obj)                              \
  if (ARCH_UNLIKELY(pystage_prop_validity_check(obj) == -1)) { \
    return NULL;                                               \
  }                                                            \
  (void)0
#define PYUSD_PROP_CHECK_INT(obj)                              \
  if (ARCH_UNLIKELY(pystage_prop_validity_check(obj) == -1)) { \
    return -1;                                                 \
  }                                                            \
  (void)0

#define PYUSD_STRUCT_IS_VALID(pystage) \
  (ARCH_LIKELY(((KPy_KrakenStage *)(pystage))->ptr->GetStage().IsInvalid() != true))
#define PYUSD_PROP_IS_VALID(pystage) \
  (ARCH_LIKELY(((KPy_KrakenPROP *)(pystage))->ptr->IsValid() == true))

#ifdef __cplusplus
}
#endif

void KPY_uni_init(void);
PyObject *KPY_uni_types(void);
PyObject *KPY_stage_module(void);

void KPY_update_stage_module(void);

KrakenPRIM *pystage_struct_as_srna(PyObject *self,
                                         const bool parent,
                                         const char *error_prefix);
PyObject *pystage_struct_CreatePyObject(KrakenPRIM *ptr);
void pystage_alloc_types(void);

PyObject *pystage_srna_PyBase(KrakenPRIM *srna);
void pystage_subtype_set_rna(PyObject *newclass, KrakenPRIM *srna);

/* kpy.utils.(un)register_class */
extern PyMethodDef meth_kpy_register_class;
extern PyMethodDef meth_kpy_unregister_class;

/* kpy.utils._bl_owner_(get/set) */
extern PyMethodDef meth_kpy_owner_id_set;
extern PyMethodDef meth_kpy_owner_id_get;

extern KPy_KrakenStage *kpy_context_module;
