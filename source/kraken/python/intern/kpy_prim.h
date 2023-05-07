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

#include "LUXO_types.h"

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
#  define USE_PYUSD_PRIM_REFERENCE

#else /* WITH_PYTHON_SAFETY */

/* default, no defines! */

#endif /* !WITH_PYTHON_SAFETY */

#define USE_PYUSD_ITER

struct ID;

#ifdef __cplusplus
extern "C" {
#endif

extern PyTypeObject pyusd_prim_meta_idprop_Type;
extern PyTypeObject pyusd_prim_Type;
extern PyTypeObject pyusd_prop_Type;
extern PyTypeObject pyusd_prop_array_Type;
extern PyTypeObject pyusd_prop_collection_Type;
extern PyTypeObject pyusd_func_Type;

#define KPy_USDPRIM_Check(v) (PyObject_TypeCheck(v, &pyusd_prim_Type))
#define KPy_USDPRIM_CheckExact(v) (Py_TYPE(v) == &pyusd_prim_Type)
#define KPy_USDPROP_Check(v) (PyObject_TypeCheck(v, &pyusd_prop_Type))
#define KPy_USDPROP_CheckExact(v) (Py_TYPE(v) == &pyusd_prop_Type)

#define PYUSD_PRIM_CHECK_OBJ(obj)                             \
  if (ARCH_UNLIKELY(pyusd_prim_validity_check(obj) == -1)) { \
    return nullptr;                                             \
  }                                                             \
  (void)0
#define PYUSD_PRIM_CHECK_INT(obj)                             \
  if (ARCH_UNLIKELY(pyusd_prim_validity_check(obj) == -1)) { \
    return -1;                                                  \
  }                                                             \
  (void)0

#define PYUSD_PROP_CHECK_OBJ(obj)                           \
  if (ARCH_UNLIKELY(pyusd_prop_validity_check(obj) == -1)) { \
    return nullptr;                                           \
  }                                                           \
  (void)0
#define PYUSD_PROP_CHECK_INT(obj)                           \
  if (ARCH_UNLIKELY(pyusd_prop_validity_check(obj) == -1)) { \
    return -1;                                                \
  }                                                           \
  (void)0

#define PYUSD_PRIM_IS_VALID(pysprim) (LIKELY(((KPy_USDPRIM *)(pysprim))->ptr.type != nullptr))
#define PYUSD_PROP_IS_VALID(pysprim) (LIKELY(((KPy_USDPROP *)(pysprim))->ptr.type != nullptr))

typedef struct
{
  PyObject_HEAD /* Required Python macro. */
#ifdef USE_WEAKREFS
    PyObject *in_weakreflist;
#endif
  KrakenPRIM ptr;
} KPy_DummyUSDPRIM;

typedef struct
{
  PyObject_HEAD /* Required Python macro. */
#ifdef USE_WEAKREFS
    PyObject *in_weakreflist;
#endif
  KrakenPRIM ptr;
#ifdef USE_PYUSD_PRIM_REFERENCE
  /**
   * generic PyObject we hold a reference to, example use:
   * hold onto the collection iterator to prevent it from
   * freeing allocated data we may use */
  PyObject *reference;
#endif /* !USE_PYUSD_PRIM_REFERENCE */

#ifdef PYUSD_FREE_SUPPORT
  bool freeptr; /* needed in some cases if ptr.data is created on the fly, free when deallocing */
#endif          /* PYUSD_FREE_SUPPORT */
} KPy_USDPRIM;

typedef struct
{
  PyObject_HEAD /* Required Python macro. */
#ifdef USE_WEAKREFS
    PyObject *in_weakreflist;
#endif
  KrakenPRIM ptr;
  KrakenPROP *prop;
} KPy_USDPROP;

typedef struct
{
  PyObject_HEAD /* Required Python macro. */
#ifdef USE_WEAKREFS
    PyObject *in_weakreflist;
#endif
  KrakenPRIM ptr;
  KrakenPROP *prop;

  /* Arystan: this is a hack to allow sub-item r/w access like: face.uv[n][m] */
  /** Array dimension, e.g: 0 for face.uv, 2 for face.uv[n][m], etc. */
  int arraydim;
  /** Array first item offset, e.g. if face.uv is [4][2], arrayoffset for face.uv[n] is 2n. */
  int arrayoffset;
} KPy_USDPROPS;

typedef struct {
  PyObject_HEAD /* Required Python macro. */
#ifdef USE_WEAKREFS
  PyObject *in_weakreflist;
#endif

  /* collection iterator specific parts */
  CollectionPropIT iter;
} KPy_USDCollectionPropIT;

typedef struct
{
  PyObject_HEAD /* Required Python macro. */
#ifdef USE_WEAKREFS
    PyObject *in_weakreflist;
#endif
  KrakenPRIM ptr;
  KrakenFUNC *func;
} KPy_USDFUNC;

KrakenPRIM *sprim_from_self(PyObject *self, const char *error_prefix);

void KPY_prim_init(void);
PyObject *KPY_prim_types(void);
PyObject *KPY_prim_module(void);
void KPY_update_prim_module(void);

KrakenPRIM *pyusd_prim_as_sprim(PyObject *self, const bool parent, const char *error_prefix);
PyObject *pyusd_prim_CreatePyObject(KrakenPRIM *ptr);
void pyprim_alloc_types(void);

void pyprim_subtype_set_prim(PyObject *newclass, KrakenPRIM *sprim);

int pyprim_deferred_register_class(KrakenPRIM *sprim, PyTypeObject *py_class);

void pyprim_invalidate(KPy_DummyUSDPRIM *self);
int pyusd_prop_validity_check(KPy_USDPROP *self);

/* kpy.utils.(un)register_class */
extern PyMethodDef meth_kpy_register_class;
extern PyMethodDef meth_kpy_unregister_class;

/* kpy.utils._bl_owner_(get/set) */
extern PyMethodDef meth_kpy_owner_id_set;
extern PyMethodDef meth_kpy_owner_id_get;

extern KPy_USDPRIM *kpy_context_module;

#ifdef __cplusplus
}
#endif
