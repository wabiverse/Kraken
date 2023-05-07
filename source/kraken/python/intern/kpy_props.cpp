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
 *
 * This file defines the 'kpy.props' module used so scripts can define their own
 * prim properties for use with python operators or adding new properties to
 * existing kraken types.
 */

/* Future-proof, See https://docs.python.org/3/c-api/arg.html#strings-and-buffers */
#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "LUXO_types.h"

#include "KLI_listbase.h"
#include "KLI_utildefines.h"

#include "kpy_capi_utils.h"
#include "kpy_props.h"
#include "kpy_prim.h"

#include "KKE_idprop.h"

#include "LUXO_access.h"
#include "LUXO_define.h" /* for defining our own prims */
#include "LUXO_enum_types.h"
// #include "LUXO_prototypes.h"

#include "MEM_guardedalloc.h"

#include "USD_ID.h" /* MAX_IDPROP_NAME */
#include "USD_listBase.h"

// #include "../generic/py_capi_prim.h"
#include "../generic/py_capi_utils.h"

/* Disabled duplicating strings because the array can still be freed and
 * the strings from it referenced, for now we can't support dynamically
 * created strings from Python. */
// #define USE_ENUM_COPY_STRINGS

/* -------------------------------------------------------------------- */
/** @name Shared Enums & Doc-Strings
 * @{ */

#define KPY_PROPDEF_OPTIONS_DOC                                            \
  "   :arg options: Enumerator in :ref:`prim_enum_property_flag_items`.\n" \
  "   :type options: set\n"

#define KPY_PROPDEF_OPTIONS_ENUM_DOC                                            \
  "   :arg options: Enumerator in :ref:`prim_enum_property_flag_enum_items`.\n" \
  "   :type options: set\n"

#define KPY_PROPDEF_OPTIONS_OVERRIDE_DOC                                             \
  "   :arg override: Enumerator in :ref:`prim_enum_property_override_flag_items`.\n" \
  "   :type override: set\n"

#define KPY_PROPDEF_OPTIONS_OVERRIDE_COLLECTION_DOC                                             \
  "   :arg override: Enumerator in :ref:`prim_enum_property_override_flag_collection_items`.\n" \
  "   :type override: set\n"

#define KPY_PROPDEF_SUBTYPE_STRING_DOC                                               \
  "   :arg subtype: Enumerator in :ref:`prim_enum_property_subtype_string_items`.\n" \
  "   :type subtype: string\n"

#define KPY_PROPDEF_SUBTYPE_NUMBER_DOC                                               \
  "   :arg subtype: Enumerator in :ref:`prim_enum_property_subtype_number_items`.\n" \
  "   :type subtype: string\n"

#define KPY_PROPDEF_SUBTYPE_NUMBER_ARRAY_DOC                                               \
  "   :arg subtype: Enumerator in :ref:`prim_enum_property_subtype_number_array_items`.\n" \
  "   :type subtype: string\n"

/** @} */

/* -------------------------------------------------------------------- */
/** \name Python Property Storage API
 *
 * Functionality needed to use Python native callbacks from generic C RNA callbacks.
 * \{ */

/**
 * Store #PyObject data for a dynamically defined property.
 * Currently this is only used to store call-back functions.
 * Properties that don't use custom callbacks won't allocate this struct.
 *
 * Memory/Reference Management
 * ---------------------------
 *
 * This struct adds/removes the user-count of each #PyObject it references,
 * it's needed in case the function is removed from the class (unlikely but possible),
 * also when an annotation evaluates to a `lambda` with Python 3.10 and newer e.g: T86332.
 *
 * Pointers to this struct are held in:
 *
 * - #PropertyRNA.py_data (owns the memory).
 *   Freed when the RNA property is freed.
 *
 * - #g_kpy_prop_store_list (borrows the memory)
 *   Having a global list means the users can be visited by the GC and cleared on exit.
 *
 *   This list can't be used for freeing as #KPyPropStore doesn't hold a #PropertyRNA back-pointer,
 *   (while it could be supported it would only complicate things).
 *
 *   All RNA properties are freed after Python has been shut-down.
 *   At that point Python user counts can't be touched and must have already been dealt with.
 *
 * Decrementing users is handled by:
 *
 * - #kpy_prop_py_data_remove manages decrementing at run-time (when a property is removed),
 *
 * - #KPY_rna_props_clear_all does this on exit for all dynamic properties.
 */
struct KPyPropStore
{
  struct KPyPropStore *next, *prev;

  /**
   * Only store #PyObject types, so this member can be cast to an array and iterated over.
   * NULL members are skipped.
   */
  struct
  {
    /** Wrap: `RNA_def_property_*_funcs` (depending on type). */
    PyObject *get_fn;
    PyObject *set_fn;
    /** Wrap: #RNA_def_property_update_runtime */
    PyObject *update_fn;

    /** Arguments by type. */
    union
    {
      /** #PROP_ENUM type. */
      struct
      {
        /** Wrap: #RNA_def_property_enum_funcs_runtime */
        PyObject *itemf_fn;
      } enum_data;
      /** #PROP_POINTER type. */
      struct
      {
        /** Wrap: #RNA_def_property_poll_runtime */
        PyObject *poll_fn;
      } pointer_data;
      /** #PROP_STRING type. */
      struct
      {
        /** Wrap: #RNA_def_property_string_search_func_runtime */
        PyObject *search_fn;
      } string_data;
    };
  } py_data;
};

#define KPY_PROP_STORE_PY_DATA_SIZE \
  (sizeof(((struct KPyPropStore *)NULL)->py_data) / sizeof(PyObject *))

#define ASSIGN_PYOBJECT_INCREF(a, b) \
  {                                  \
    KLI_assert((a) == NULL);         \
    Py_INCREF(b);                    \
    a = b;                           \
  }                                  \
  ((void)0)

/**
 * Maintain a list of Python defined properties, so the GC can visit them,
 * and so they can be cleared on exit.
 */
static ListBase g_kpy_prop_store_list = {NULL, NULL};

static struct KPyPropStore *kpy_prop_py_data_ensure(struct KrakenPROP *prop)
{
  struct KPyPropStore *prop_store = static_cast<KPyPropStore *>(LUXO_prop_py_data_get(prop));
  if (prop_store == NULL) {
    prop_store = static_cast<KPyPropStore *>(MEM_callocN(sizeof(*prop_store), __func__));
    PRIM_def_py_data(prop, prop_store);
    KLI_addtail(&g_kpy_prop_store_list, prop_store);
  }
  return prop_store;
}

/**
 * Perform all removal actions except for freeing, which is handled by RNA.
 */
static void kpy_prop_py_data_remove(KrakenPROP *prop)
{
  struct KPyPropStore *prop_store = static_cast<KPyPropStore *>(LUXO_prop_py_data_get(prop));
  if (prop_store == NULL) {
    return;
  }

  PyObject **py_data = (PyObject **)&prop_store->py_data;
  for (int i = 0; i < KPY_PROP_STORE_PY_DATA_SIZE; i++) {
    Py_XDECREF(py_data[i]);
  }
  KLI_remlink(&g_kpy_prop_store_list, prop_store);
}

/** @} */

/* -------------------------------------------------------------------- */
/** @name Main Module `kpy.props`
 * @{ */

static int props_visit(PyObject *UNUSED(self), visitproc visit, void *arg)
{
  LISTBASE_FOREACH(struct KPyPropStore *, prop_store, &g_kpy_prop_store_list)
  {
    PyObject **py_data = (PyObject **)&prop_store->py_data;
    for (int i = 0; i < KPY_PROP_STORE_PY_DATA_SIZE; i++) {
      Py_VISIT(py_data[i]);
    }
  }
  return 0;
}

static int props_clear(PyObject *UNUSED(self))
{
  LISTBASE_FOREACH(struct KPyPropStore *, prop_store, &g_kpy_prop_store_list)
  {
    PyObject **py_data = (PyObject **)&prop_store->py_data;
    for (int i = 0; i < KPY_PROP_STORE_PY_DATA_SIZE; i++) {
      Py_CLEAR(py_data[i]);
    }
  }
  return 0;
}

static struct PyModuleDef props_module = {
  PyModuleDef_HEAD_INIT,
  "kpy.props",
  "This module defines properties to extend Kraken's internal data. The result of these "
  "functions"
  " is used to assign properties to classes registered with Kraken and can't be used "
  "directly.\n"
  "\n"
  ".. note:: All parameters to these functions must be passed as keywords.\n",
  -1, /* multiple "initialization" just copies the module dict. */
  /** @TODO: props_methods */ NULL,
  NULL,
  props_visit,
  props_clear,
  NULL,
};

static PyGetSetDef kpy_prop_deferred_getset[] = {
    // {"function", (getter)kpy_prop_deferred_function_get, (setter)NULL, NULL, NULL},
    // {"keywords", (getter)kpy_prop_deferred_keywords_get, (setter)NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL} /* Sentinel */
};

PyDoc_STRVAR(kpy_prop_deferred_doc,
             "Intermediate storage for properties before registration.\n"
             "\n"
             ".. note::\n"
             "\n"
             "   This is not part of the stable API and may change between releases.");

PyTypeObject kpy_prop_deferred_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)

  .tp_name = "_PropertyDeferred",
  .tp_basicsize = sizeof(KPy_PropDeferred),
  // .tp_dealloc = (destructor)kpy_prop_deferred_dealloc,
  // .tp_repr = (reprfunc)kpy_prop_deferred_repr,
  // .tp_call = (ternaryfunc)kpy_prop_deferred_call,

  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,

  .tp_doc = kpy_prop_deferred_doc,
  // .tp_traverse = (traverseproc)kpy_prop_deferred_traverse,
  // .tp_clear = (inquiry)kpy_prop_deferred_clear,

  .tp_getset = kpy_prop_deferred_getset,
};

PyObject *KPY_prim_props(void)
{
  PyObject *submodule;
  PyObject *submodule_dict;

  submodule = PyModule_Create(&props_module);
  PyDict_SetItemString(PyImport_GetModuleDict(), props_module.m_name, submodule);

  /* api needs the PyObjects internally */
  submodule_dict = PyModule_GetDict(submodule);

#define ASSIGN_STATIC(_name) pymeth_##_name = PyDict_GetItemString(submodule_dict, #_name)

  // ASSIGN_STATIC(BoolProperty);
  // ASSIGN_STATIC(BoolVectorProperty);
  // ASSIGN_STATIC(IntProperty);
  // ASSIGN_STATIC(IntVectorProperty);
  // ASSIGN_STATIC(FloatProperty);
  // ASSIGN_STATIC(FloatVectorProperty);
  // ASSIGN_STATIC(StringProperty);
  // ASSIGN_STATIC(EnumProperty);
  // ASSIGN_STATIC(PointerProperty);
  // ASSIGN_STATIC(CollectionProperty);
  // ASSIGN_STATIC(RemoveProperty);

  if (PyType_Ready(&kpy_prop_deferred_Type) < 0) {
    return NULL;
  }
  PyModule_AddType(submodule, &kpy_prop_deferred_Type);

  /* Run this when properties are freed. */
  PRIM_def_property_free_pointers_set_py_data_callback(kpy_prop_py_data_remove);

  return submodule;
}

void KPY_prim_props_clear_all(void)
{
  /* Remove all user counts, so this isn't considered a leak from Python's perspective. */
  props_clear(NULL);

  /* Running is harmless, but redundant. */
  PRIM_def_property_free_pointers_set_py_data_callback(NULL);

  /* Include as it's correct, in practice this should never be used again. */
  KLI_listbase_clear(&g_kpy_prop_store_list);
}

/** @} */
