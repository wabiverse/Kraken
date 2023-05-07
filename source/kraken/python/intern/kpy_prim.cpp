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

#include <Python.h>

#include "CLG_log.h"

#include <float.h> /* FLT_MIN/MAX */
#include <stddef.h>

#include "KLI_rhash.h"
#include "KLI_listbase.h"
#include "KLI_utildefines.h"
#include "KLI_string.h"

#include "KKE_context.h"
#include "KKE_idprop.h"
#include "KKE_idtype.h"
#include "KKE_main.h"
#include "KKE_robinhood.h"
#include "KKE_utils.h"
#include "KKE_report.h"
#include "KKE_global.h"

#include "LUXO_access.h"
#include "LUXO_define.h"
#include "LUXO_enum_types.h"
// #include "LUXO_prototypes.h"

#include "USD_wm_types.h"
#include "USD_types.h"
#include "USD_object.h"

#include "KPY_extern.h"
#include "KPY_extern_clog.h"

#include "../generic/py_capi_utils.h"
#include "../generic/python_utildefines.h"

#include "kpy_interface.h"
#include "kpy_intern_string.h"
#include "kpy_prim.h"
#include "kpy_props.h"
#include "kpy_capi_utils.h"

#include <wabi/usd/usd/prim.h>
#include <wabi/base/tf/iterator.h>

#include <boost/python.hpp>
#include <boost/python/overloads.hpp>

using namespace boost::python;
using namespace wabi;
using namespace kraken;

#define MAX_ARRAY_DIMENSION 10

#define USE_PEDANTIC_WRITE
// #define USE_MATHUTILS
#define USE_STRING_COERCE

/**
 * This _must_ be enabled to support Python 3.10's postponed annotations,
 * `from __future__ import annotations`.
 *
 * This has the disadvantage of evaluating strings at run-time, in the future we might be able to
 * reinstate the older, more efficient logic using descriptors, see: pep-0649
 */
#define USE_POSTPONED_ANNOTATIONS

KPy_StagePRIM *kpy_context_module = nullptr; /* for fast access */

static PyObject *pyprim_prim_Subtype(KrakenPRIM *ptr);

static PyObject *pyprim_register_class(PyObject *self, PyObject *py_class);
static PyObject *pyprim_unregister_class(PyObject *self, PyObject *py_class);

static PyObject *pyprim_prop_to_py(KrakenPRIM *ptr, KrakenPROP *prop);

#define KPY_DOC_ID_PROP_TYPE_NOTE                                       \
  "   .. note::\n"                                                      \
  "\n"                                                                  \
  "      Only the :class:`kpy.types.ID`, :class:`kpy.types.Bone` and\n" \
  "      :class:`kpy.types.PoseBone` classes support custom properties.\n"

int pyprim_prim_validity_check(KPy_StagePRIM *pysprim)
{
  if (pysprim->ptr.type) {
    return 0;
  }
  PyErr_Format(PyExc_ReferenceError,
               "StructRNA of type %.200s has been removed",
               Py_TYPE(pysprim)->tp_name);
  return -1;
}

int pyprim_prop_validity_check(KPy_StagePROP *self)
{
  if (self->ptr.type) {
    return 0;
  }
  PyErr_Format(PyExc_ReferenceError,
               "KrakenPROP of type %.200s.%.200s has been removed",
               Py_TYPE(self)->tp_name,
               LUXO_prop_identifier(self->prop).data());
  return -1;
}

void pyprim_invalidate(KPy_DummyStagePRIM *self)
{
  PRIM_POINTER_INVALIDATE(&self->ptr);
}

#ifdef USE_PEDANTIC_WRITE
static bool prim_disallow_writes = false;

static bool prim_id_write_error(KrakenPRIM *ptr, PyObject *key)
{
  ID *id = ptr->owner_id;
  if (id) {
    const short idcode = GS(id->name);
    /* May need more ID types added here. */
    if (!ELEM(idcode, ID_WM, ID_SCR, ID_WS)) {
      const char *idtype = KKE_idtype_idcode_to_name(idcode);
      const char *pyname;
      if (key && PyUnicode_Check(key)) {
        pyname = PyUnicode_AsUTF8(key);
      } else {
        pyname = "<UNKNOWN>";
      }

      /* Make a nice string error. */
      KLI_assert(idtype != NULL);
      PyErr_Format(PyExc_AttributeError,
                   "Writing to ID classes in this context is not allowed: "
                   "%.200s, %.200s datablock, error setting %.200s.%.200s",
                   id->name + 2,
                   idtype,
                   LUXO_prim_identifier(ptr->type).data(),
                   pyname);

      return true;
    }
  }
  return false;
}
#endif /* USE_PEDANTIC_WRITE */

#ifdef USE_PEDANTIC_WRITE
bool pyprim_write_check(void)
{
  return !prim_disallow_writes;
}

void pyprim_write_set(bool val)
{
  prim_disallow_writes = !val;
}
#else  /* USE_PEDANTIC_WRITE */
bool pyprim_write_check(void)
{
  return true;
}

void pyprim_write_set(bool UNUSED(val))
{
  /* pass */
}
#endif /* USE_PEDANTIC_WRITE */

static PyObject *pyprim_prop_CreatePyObject(KrakenPRIM *ptr, KrakenPROP *prop);

static Py_ssize_t pyprim_prop_collection_length(KPy_StagePROP *self);
static Py_ssize_t pyprim_prop_array_length(KPy_StagePropARRAY *self);
static int pyprim_py_to_prop(KrakenPRIM *ptr,
                             KrakenPROP *prop,
                             void *data,
                             PyObject *value,
                             const char *error_prefix);
static int deferred_register_prop(KrakenPRIM *sprim, PyObject *key, PyObject *item);

/* Use our own dealloc so we can free a property if we use one. */
static void pyprim_prop_dealloc(KPy_StagePROP *self)
{
#ifdef USE_WEAKREFS
  if (self->in_weakreflist != NULL) {
    PyObject_ClearWeakRefs((PyObject *)self);
  }
#endif
  /* NOTE: for subclassed PyObjects calling PyObject_DEL() directly crashes. */
  Py_TYPE(self)->tp_free(self);
}

static PyObject *pyprim_prop_str(KPy_StagePROP *self)
{
  PyObject *ret;
  KrakenPRIM ptr;
  const char *name;
  const char *type_id = NULL;
  char type_fmt[64] = "";
  int type;

  PYSTAGE_PROP_CHECK_OBJ(self);

  type = LUXO_prop_type(self->prop);

  if (LUXO_enum_id_from_value(prim_enum_prop_type_items, type, &type_id) == 0) {
    /* Should never happen. */
    PyErr_SetString(PyExc_RuntimeError, "could not use property type, internal error");
    return NULL;
  }

  /* This should never fail. */
  int len = -1;
  char *c = type_fmt;

  while ((*c++ = tolower(*type_id++))) {
  }

  if (type == PROP_COLLECTION) {
    len = pyprim_prop_collection_length(self);
  } else if (LUXO_prop_array_check(self->prop)) {
    len = pyprim_prop_array_length((KPy_StagePropARRAY *)self);
  }

  if (len != -1) {
    sprintf(--c, "[%d]", len);
  }

  /* If a pointer, try to print name of pointer target too. */
  if (type == PROP_POINTER) {
    ptr = LUXO_prop_pointer_get(&self->ptr, self->prop);
    name = ptr.GetName().GetText();

    if (name) {
      ret = PyUnicode_FromFormat("<kpy_%.200s, %.200s.%.200s(\"%.200s\")>",
                                 type_fmt,
                                 LUXO_prim_identifier(self->ptr.type).data(),
                                 LUXO_prop_identifier(self->prop).data(),
                                 name);
      MEM_freeN((void *)name);
      return ret;
    }
  }
  if (type == PROP_COLLECTION) {
    KrakenPRIM r_ptr;
    if (LUXO_prop_collection_type_get(&self->ptr, self->prop, &r_ptr)) {
      KrakenPROP *prop;
      *prop = r_ptr.type->GetAttribute(TfToken(type_fmt));
      return PyUnicode_FromFormat("<kpy_%.200s, %.200s>",
                                  type_fmt,
                                  LUXO_prop_identifier(prop).data());
    }
  }

  return PyUnicode_FromFormat("<kpy_%.200s, %.200s.%.200s>",
                              type_fmt,
                              LUXO_prim_identifier(self->ptr.type).data(),
                              LUXO_prop_identifier(self->prop).data());
}

static PyObject *pyprim_prop_repr_ex(KPy_StagePROP *self, const int index_dim, const int index)
{
  ID *id = self->ptr.owner_id;
  PyObject *tmp_str;
  PyObject *ret;
  const char *path;

  PYSTAGE_PROP_CHECK_OBJ(self);

  if (id == NULL) {
    /* Fallback. */
    return pyprim_prop_str(self);
  }

  tmp_str = PyUnicode_FromString(id->name + 2);

  /* Note that using G_MAIN is absolutely not ideal, but we have no access to actual Main DB from
   * here. */
  ID *real_id = NULL;
  path = LUXO_path_from_real_ID_to_property_index(G_MAIN,
                                                  &self->ptr,
                                                  self->prop,
                                                  index_dim,
                                                  index,
                                                  &real_id);

  if (path) {
    if (real_id != id) {
      Py_DECREF(tmp_str);
      tmp_str = PyUnicode_FromString(real_id->name + 2);
    }
    const char *data_delim = (path[0] == '[') ? "" : ".";
    ret = PyUnicode_FromFormat("kpy.data.%s[%R]%s%s",
                               KKE_idtype_idcode_to_name_plural(GS(real_id->name)),
                               tmp_str,
                               data_delim,
                               path);

    MEM_freeN((void *)path);
  } else {
    /* Can't find the path, print something useful as a fallback. */
    ret = PyUnicode_FromFormat("kpy.data.%s[%R]...%s",
                               KKE_idtype_idcode_to_name_plural(GS(id->name)),
                               tmp_str,
                               LUXO_prop_identifier(self->prop).data());
  }

  Py_DECREF(tmp_str);

  return ret;
}

static PyObject *pyprim_prop_repr(KPy_StagePROP *self)
{
  return pyprim_prop_repr_ex(self, 0, -1);
}

static PyObject *pyprim_prop_array_repr(KPy_StagePropARRAY *self)
{
  return pyprim_prop_repr_ex((KPy_StagePROP *)self, self->arraydim, self->arrayoffset);
}

static PyObject *pyprim_func_repr(KPy_KrakenFUNC *self)
{
  return PyUnicode_FromFormat("<%.200s %.200s.%.200s()>",
                              Py_TYPE(self)->tp_name,
                              LUXO_prim_identifier(self->ptr.type).data(),
                              LUXO_function_identifier(self->func));
}

PyObject *pyprim_array_index(KrakenPRIM *ptr, KrakenPROP *prop, int index)
{
  PyObject *item;

  switch (LUXO_prop_type(prop)) {
    case PROP_FLOAT:
      item = PyFloat_FromDouble(LUXO_prop_float_get_index(ptr, prop, index));
      break;
    case PROP_BOOLEAN:
      item = PyBool_FromLong(LUXO_prop_boolean_get_index(ptr, prop, index));
      break;
    case PROP_INT:
      item = PyLong_FromLong(LUXO_prop_int_get_index(ptr, prop, index));
      break;
    default:
      PyErr_SetString(PyExc_TypeError, "not an array type");
      item = NULL;
      break;
  }

  return item;
}

PyObject *pyprim_pyprim(KPy_StagePropARRAY *self, KrakenPRIM *ptr, KrakenPROP *prop, int index)
{
  int totdim, arraydim, arrayoffset, dimsize[MAX_ARRAY_DIMENSION], i, len;
  KPy_StagePropARRAY *ret = NULL;

  arraydim = self ? self->arraydim : 0;
  arrayoffset = self ? self->arrayoffset : 0;

  /* just in case check */
  len = LUXO_prop_multi_array_length(ptr, prop, arraydim);
  if (index >= len || index < 0) {
    /* This shouldn't happen because higher level functions must check for invalid index. */
    CLOG_WARN(KPY_LOG_PRIM, "invalid index %d for array with length=%d", index, len);

    PyErr_SetString(PyExc_IndexError, "out of range");
    return NULL;
  }

  totdim = LUXO_prop_array_dimension(ptr, prop, dimsize);

  if (arraydim + 1 < totdim) {
    ret = (KPy_StagePropARRAY *)pyprim_prop_CreatePyObject(ptr, prop);
    ret->arraydim = arraydim + 1;

    /* arr[3][4][5]
     *
     *    x = arr[2]
     *    index = 0 + 2 * 4 * 5
     *
     *    x = arr[2][3]
     *    index = offset + 3 * 5 */

    for (i = arraydim + 1; i < totdim; i++) {
      index *= dimsize[i];
    }

    ret->arrayoffset = arrayoffset + index;
  } else {
    index = arrayoffset + index;
    ret = (KPy_StagePropARRAY *)pyprim_array_index(ptr, prop, index);
  }

  return (PyObject *)ret;
}

static PyObject *pyprim_prop_array_to_py_index(KPy_StagePropARRAY *self, int index)
{
  PYSTAGE_PROP_CHECK_OBJ((KPy_StagePROP *)self);
  return pyprim_pyprim(self, &self->ptr, self->prop, index);
}

/* bool functions are for speed, so we can avoid getting the length
 * of 1000's of items in a linked list for eg. */
static int pyprim_prop_array_bool(KPy_StagePROP *self)
{
  PYSTAGE_PROP_CHECK_INT(self);

  return LUXO_prop_array_length(&self->ptr, self->prop) ? 1 : 0;
}

static int pyprim_prop_collection_bool(KPy_StagePROP *self)
{
  PYSTAGE_PROP_CHECK_INT(self);

  return !LUXO_prop_collection_is_empty(&self->ptr, self->prop);
}

/* notice getting the length of the collection is avoided unless negative
 * index is used or to detect internal error with a valid index.
 * This is done for faster lookups. */
#define PYPRIM_PROP_COLLECTION_ABS_INDEX(ret_err)                                       \
  if (keynum < 0) {                                                                     \
    keynum_abs += LUXO_prop_collection_length(&self->ptr, self->prop);                  \
    if (keynum_abs < 0) {                                                               \
      PyErr_Format(PyExc_IndexError, "kpy_prop_collection[%d]: out of range.", keynum); \
      return ret_err;                                                                   \
    }                                                                                   \
  }                                                                                     \
  (void)0

/* Values type must have been already checked. */
static int pyprim_prop_collection_ass_subscript_int(KPy_StagePROP *self,
                                                    Py_ssize_t keynum,
                                                    PyObject *value)
{
  Py_ssize_t keynum_abs = keynum;
  const KrakenPRIM *ptr = (value == Py_None) ? (&KrakenPRIM_NULL) : &((KPy_StagePRIM *)value)->ptr;

  PYSTAGE_PROP_CHECK_INT(self);

  PYPRIM_PROP_COLLECTION_ABS_INDEX(-1);

  if (LUXO_prop_collection_assign_int(&self->ptr, self->prop, keynum_abs, ptr) == 0) {
    const int len = LUXO_prop_collection_length(&self->ptr, self->prop);
    if (keynum_abs >= len) {
      PyErr_Format(PyExc_IndexError,
                   "kpy_prop_collection[index] = value: "
                   "index %d out of range, size %d",
                   keynum,
                   len);
    } else {

      PyErr_Format(PyExc_IndexError,
                   "kpy_prop_collection[index] = value: "
                   "failed assignment (unknown reason)",
                   keynum);
    }
    return -1;
  }

  return 0;
}

static PyObject *pyprim_prop_array_subscript_int(KPy_StagePropARRAY *self, int keynum)
{
  int len;

  PYSTAGE_PROP_CHECK_OBJ((KPy_StagePROP *)self);

  len = pyprim_prop_array_length(self);

  if (keynum < 0) {
    keynum += len;
  }

  if (keynum >= 0 && keynum < len) {
    return pyprim_prop_array_to_py_index(self, keynum);
  }

  PyErr_Format(PyExc_IndexError, "kpy_prop_array[index]: index %d out of range", keynum);
  return NULL;
}

/**
 * Special case: `kpy.data.objects["some_id_name", "//some_lib_name.usd"]`
 * also for:     `kpy.data.objects.get(("some_id_name", "//some_lib_name.usd"), fallback)`
 *
 * \note
 * error codes since this is not to be called directly from Python,
 * this matches Python's `__contains__` values C-API.
 * - -1: exception set
 * -  0: not found
 * -  1: found
 */
static int pyprim_prop_collection_subscript_str_lib_pair_ptr(KPy_StagePROP *self,
                                                             PyObject *key,
                                                             const char *err_prefix,
                                                             const short err_not_found,
                                                             KrakenPRIM *r_ptr)
{
  const char *keyname;

  /* First validate the args, all we know is that they are a tuple. */
  if (PyTuple_GET_SIZE(key) != 2) {
    PyErr_Format(PyExc_KeyError,
                 "%s: tuple key must be a pair, not size %d",
                 err_prefix,
                 PyTuple_GET_SIZE(key));
    return -1;
  }
  if (self->ptr.type != &PRIM_StageData) {
    PyErr_Format(PyExc_KeyError,
                 "%s: is only valid for kpy.data collections, not %.200s",
                 err_prefix,
                 LUXO_prim_identifier(self->ptr.type).data());
    return -1;
  }
  if ((keyname = PyUnicode_AsUTF8(PyTuple_GET_ITEM(key, 0))) == NULL) {
    PyErr_Format(PyExc_KeyError,
                 "%s: id must be a string, not %.200s",
                 err_prefix,
                 Py_TYPE(PyTuple_GET_ITEM(key, 0))->tp_name);
    return -1;
  }

  PyObject *keylib = PyTuple_GET_ITEM(key, 1);
  Library *lib;
  bool found = false;

  if (keylib == Py_None) {
    lib = NULL;
  } else if (PyUnicode_Check(keylib)) {
    Main *bmain = static_cast<Main *>(self->ptr.data);
    const char *keylib_str = PyUnicode_AsUTF8(keylib);
    lib = static_cast<Library *>(
      KLI_findstring(&bmain->libraries, keylib_str, offsetof(Library, filepath)));
    if (lib == NULL) {
      if (err_not_found) {
        PyErr_Format(PyExc_KeyError,
                     "%s: lib filepath '%.1024s' "
                     "does not reference a valid library",
                     err_prefix,
                     keylib_str);
        return -1;
      }

      return 0;
    }
  } else {
    PyErr_Format(PyExc_KeyError,
                 "%s: lib must be a string or None, not %.200s",
                 err_prefix,
                 Py_TYPE(keylib)->tp_name);
    return -1;
  }

  /* lib is either a valid pointer or NULL,
   * either way can do direct comparison with id.lib */

  LUXO_PROP_BEGIN(&self->ptr, itemptr, self->prop)
  {
    ID *id = static_cast<ID *>(itemptr.data); /* Always an ID. */
    if (id->lib == lib && STREQLEN(keyname, id->name + 2, sizeof(id->name) - 2)) {
      found = true;
      if (r_ptr) {
        *r_ptr = itemptr;
      }
      break;
    }
  }
  LUXO_PROP_END;

  /* We may want to fail silently as with collection.get(). */
  if ((found == false) && err_not_found) {
    /* Only runs for getitem access so use fixed string. */
    PyErr_SetString(PyExc_KeyError, "kpy_prop_collection[key, lib]: not found");
    return -1;
  }

  return found; /* 1 / 0, no exception. */
}

static PyObject *pyprim_prop_collection_subscript_str_lib_pair(KPy_StagePROP *self,
                                                               PyObject *key,
                                                               const char *err_prefix,
                                                               const bool err_not_found)
{
  KrakenPRIM ptr;
  const int contains =
    pyprim_prop_collection_subscript_str_lib_pair_ptr(self, key, err_prefix, err_not_found, &ptr);

  if (contains == 1) {
    return pyprim_prim_CreatePyObject(&ptr);
  }

  return NULL;
}

static PyObject *pyprim_prop_collection_subscript_slice(KPy_StagePROP *self,
                                                        Py_ssize_t start,
                                                        Py_ssize_t stop)
{
  CollectionPropIT rna_macro_iter;
  int count;

  PyObject *list;
  PyObject *item;

  PYSTAGE_PROP_CHECK_OBJ(self);

  list = PyList_New(0);

  /* Skip to start. */
  LUXO_prop_collection_begin(&self->ptr, self->prop, &rna_macro_iter);
  LUXO_prop_collection_skip(&rna_macro_iter, start);

  /* Add items until stop. */
  for (count = start; rna_macro_iter.valid; LUXO_prop_collection_next(&rna_macro_iter)) {
    item = pyprim_prim_CreatePyObject(&rna_macro_iter.ptr);
    PyList_APPEND(list, item);

    count++;
    if (count == stop) {
      break;
    }
  }

  LUXO_prop_collection_end(&rna_macro_iter);

  return list;
}

/**
 * TODO: dimensions
 * @note Could also use pyprim_prop_array_to_py_index(self, count) in a loop, but it's much slower
 * since at the moment it reads (and even allocates) the entire array for each index.
 */
static PyObject *pyprim_prop_array_subscript_slice(KPy_StagePropARRAY *self,
                                                   KrakenPRIM *ptr,
                                                   KrakenPROP *prop,
                                                   Py_ssize_t start,
                                                   Py_ssize_t stop,
                                                   Py_ssize_t length)
{
  int count, totdim;
  PyObject *tuple;

  /* Isn't needed, internal use only. */
  // PYLUXO_PROP_CHECK_OBJ((KPy_KrakenPROP *)self);

  tuple = PyTuple_New(stop - start);

  totdim = LUXO_prop_array_dimension(ptr, prop, NULL);

  if (totdim > 1) {
    for (count = start; count < stop; count++) {
      PyTuple_SET_ITEM(tuple, count - start, pyprim_prop_array_to_py_index(self, count));
    }
  } else {
    switch (LUXO_prop_type(prop)) {
      case PROP_FLOAT: {
        float values_stack[PYPRIM_STACK_ARRAY];
        float *values;
        if (length > PYPRIM_STACK_ARRAY) {
          values = (float *)PyMem_MALLOC(sizeof(float) * length);
        } else {
          values = values_stack;
        }
        LUXO_prop_float_get_array(ptr, prop, values);

        for (count = start; count < stop; count++) {
          PyTuple_SET_ITEM(tuple, count - start, PyFloat_FromDouble(values[count]));
        }

        if (values != values_stack) {
          PyMem_FREE(values);
        }
        break;
      }
      case PROP_BOOLEAN: {
        bool values_stack[PYPRIM_STACK_ARRAY];
        bool *values;
        if (length > PYPRIM_STACK_ARRAY) {
          values = static_cast<bool *>(PyMem_MALLOC(sizeof(bool) * length));
        } else {
          values = values_stack;
        }

        LUXO_prop_boolean_get_array(ptr, prop, values);
        for (count = start; count < stop; count++) {
          PyTuple_SET_ITEM(tuple, count - start, PyBool_FromLong(values[count]));
        }

        if (values != values_stack) {
          PyMem_FREE(values);
        }
        break;
      }
      case PROP_INT: {
        int values_stack[PYPRIM_STACK_ARRAY];
        int *values;
        if (length > PYPRIM_STACK_ARRAY) {
          values = static_cast<int *>(PyMem_MALLOC(sizeof(int) * length));
        } else {
          values = values_stack;
        }

        LUXO_prop_int_get_array(ptr, prop, values);
        for (count = start; count < stop; count++) {
          PyTuple_SET_ITEM(tuple, count - start, PyLong_FromLong(values[count]));
        }

        if (values != values_stack) {
          PyMem_FREE(values);
        }
        break;
      }
      default:
        KLI_assert_msg(0, "Invalid array type");

        PyErr_SetString(PyExc_TypeError, "not an array type");
        Py_DECREF(tuple);
        tuple = NULL;
        break;
    }
  }
  return tuple;
}

static PyObject *pyprim_prop_array_subscript(KPy_StagePropARRAY *self, PyObject *key)
{
  PYSTAGE_PROP_CHECK_OBJ((KPy_StagePROP *)self);

#if 0
  if (PyUnicode_Check(key)) {
    return pyprim_prop_array_subscript_str(self, PyUnicode_AsUTF8(key));
  }
  else
#endif
  if (PyIndex_Check(key)) {
    const Py_ssize_t i = PyNumber_AsSsize_t(key, PyExc_IndexError);
    if (i == -1 && PyErr_Occurred()) {
      return NULL;
    }
    return pyprim_prop_array_subscript_int(self, i);
  }
  if (PySlice_Check(key)) {
    Py_ssize_t step = 1;
    PySliceObject *key_slice = (PySliceObject *)key;

    if (key_slice->step != Py_None && !_PyEval_SliceIndex(key, &step)) {
      return NULL;
    }
    if (step != 1) {
      PyErr_SetString(PyExc_TypeError, "kpy_prop_array[slice]: slice steps not supported");
      return NULL;
    }
    if (key_slice->start == Py_None && key_slice->stop == Py_None) {
      /* NOTE: no significant advantage with optimizing [:] slice as with collections,
       * but include here for consistency with collection slice func */
      const Py_ssize_t len = (Py_ssize_t)pyprim_prop_array_length(self);
      return pyprim_prop_array_subscript_slice(self, &self->ptr, self->prop, 0, len, len);
    }

    const int len = pyprim_prop_array_length(self);
    Py_ssize_t start, stop, slicelength;

    if (PySlice_GetIndicesEx(key, len, &start, &stop, &step, &slicelength) < 0) {
      return NULL;
    }

    if (slicelength <= 0) {
      return PyTuple_New(0);
    }

    return pyprim_prop_array_subscript_slice(self, &self->ptr, self->prop, start, stop, len);
  }

  PyErr_SetString(PyExc_AttributeError, "kpy_prop_array[key]: invalid key, key must be an int");
  return NULL;
}

/**
 * @param result: The result of calling a subscription operation on a collection (never NULL).
 */
static int pyprim_prop_collection_subscript_is_valid_or_error(const PyObject *value)
{
  if (value != Py_None) {
    KLI_assert(KPy_StagePRIM_Check(value));
    const KPy_StagePRIM *value_pyprim = (const KPy_StagePRIM *)value;
    if (UNLIKELY(value_pyprim->ptr.type == NULL)) {
      /* It's important to use a `TypeError` as that is what's returned when `__getitem__` is
       * called on an object that doesn't support item access. */
      PyErr_Format(PyExc_TypeError,
                   "'%.200s' object is not subscriptable (only iteration is supported)",
                   Py_TYPE(value)->tp_name);
      return -1;
    }
  }
  return 0;
}

static PyObject *pyprim_prop_collection_subscript_str(KPy_StagePROP *self, const char *keyname)
{
  KrakenPRIM newptr;

  PYSTAGE_PROP_CHECK_OBJ(self);

  if (LUXO_prop_collection_lookup_string_has_fn(self->prop)) {
    if (LUXO_prop_collection_lookup_string(&self->ptr, self->prop, keyname, &newptr)) {
      return pyprim_prim_CreatePyObject(&newptr);
    }
  } else {
    /* No callback defined, just iterate and find the nth item. */
    const int keylen = strlen(keyname);
    char name[256];
    int namelen;
    PyObject *result = NULL;
    bool found = false;
    CollectionPropIT iter;
    LUXO_prop_collection_begin(&self->ptr, self->prop, &iter);
    for (int i = 0; iter.valid; LUXO_prop_collection_next(&iter), i++) {
      KrakenPROP nameprop = iter.ptr.type->GetAttribute(TfToken("name"));
      char *nameptr = KLI_strdup(nameprop.GetName().data());
      if ((keylen == namelen) && STREQ(nameptr, keyname)) {
        found = true;
      }
      if ((char *)&name != nameptr) {
        MEM_freeN(nameptr);
      }
      if (found) {
        result = pyprim_prim_CreatePyObject(&iter.ptr);
        break;
      }
    }
    /* It's important to end the iterator after `result` has been created
     * so iterators may optionally invalidate items that were iterated over, see: T100286. */
    LUXO_prop_collection_end(&iter);
    if (found) {
      if (result && (pyprim_prop_collection_subscript_is_valid_or_error(result) == -1)) {
        Py_DECREF(result);
        result = NULL; /* The exception has been set. */
      }
      return result;
    }
  }

  PyErr_Format(PyExc_KeyError, "kpy_prop_collection[key]: key \"%.200s\" not found", keyname);
  return NULL;
}

/* Internal use only. */
static PyObject *pyprim_prop_collection_subscript_int(KPy_StagePROP *self, Py_ssize_t keynum)
{
  KrakenPRIM newptr;
  Py_ssize_t keynum_abs = keynum;

  PYSTAGE_PROP_CHECK_OBJ(self);

  PYPRIM_PROP_COLLECTION_ABS_INDEX(NULL);

  if (LUXO_prop_collection_lookup_int_has_fn(self->prop)) {
    if (LUXO_prop_collection_lookup_int(&self->ptr, self->prop, keynum_abs, &newptr)) {
      return pyprim_prim_CreatePyObject(&newptr);
    }
  } else {
    /* No callback defined, just iterate and find the nth item. */
    const int key = (int)keynum_abs;
    PyObject *result = NULL;
    bool found = false;
    CollectionPropIT iter;
    LUXO_prop_collection_begin(&self->ptr, self->prop, &iter);
    for (int i = 0; iter.valid; LUXO_prop_collection_next(&iter), i++) {
      if (i == key) {
        result = pyprim_prim_CreatePyObject(&iter.ptr);
        found = true;
        break;
      }
    }
    /* It's important to end the iterator after `result` has been created
     * so iterators may optionally invalidate items that were iterated over, see: T100286. */
    LUXO_prop_collection_end(&iter);
    if (found) {
      if (result && (pyprim_prop_collection_subscript_is_valid_or_error(result) == -1)) {
        Py_DECREF(result);
        result = NULL; /* The exception has been set. */
      }
      return result;
    }
  }

  const int len = LUXO_prop_collection_length(&self->ptr, self->prop);
  if (keynum_abs >= len) {
    PyErr_Format(PyExc_IndexError,
                 "kpy_prop_collection[index]: "
                 "index %d out of range, size %d",
                 keynum,
                 len);
  } else {
    PyErr_Format(PyExc_RuntimeError,
                 "kpy_prop_collection[index]: internal error, "
                 "valid index %d given in %d sized collection, but value not found",
                 keynum_abs,
                 len);
  }

  return NULL;
}

static PyObject *pyprim_prop_collection_subscript(KPy_StagePROP *self, PyObject *key)
{
  PYSTAGE_PROP_CHECK_OBJ(self);

  if (PyUnicode_Check(key)) {
    return pyprim_prop_collection_subscript_str(self, PyUnicode_AsUTF8(key));
  }
  if (PyIndex_Check(key)) {
    const Py_ssize_t i = PyNumber_AsSsize_t(key, PyExc_IndexError);
    if (i == -1 && PyErr_Occurred()) {
      return NULL;
    }

    return pyprim_prop_collection_subscript_int(self, i);
  }
  if (PySlice_Check(key)) {
    PySliceObject *key_slice = (PySliceObject *)key;
    Py_ssize_t step = 1;

    if (key_slice->step != Py_None && !_PyEval_SliceIndex(key, &step)) {
      return NULL;
    }
    if (step != 1) {
      PyErr_SetString(PyExc_TypeError, "kpy_prop_collection[slice]: slice steps not supported");
      return NULL;
    }
    if (key_slice->start == Py_None && key_slice->stop == Py_None) {
      return pyprim_prop_collection_subscript_slice(self, 0, PY_SSIZE_T_MAX);
    }

    Py_ssize_t start = 0, stop = PY_SSIZE_T_MAX;

    /* Avoid PySlice_GetIndicesEx because it needs to know the length ahead of time. */
    if (key_slice->start != Py_None && !_PyEval_SliceIndex(key_slice->start, &start)) {
      return NULL;
    }
    if (key_slice->stop != Py_None && !_PyEval_SliceIndex(key_slice->stop, &stop)) {
      return NULL;
    }

    if (start < 0 || stop < 0) {
      /* Only get the length for negative values. */
      const Py_ssize_t len = (Py_ssize_t)LUXO_prop_collection_length(&self->ptr, self->prop);
      if (start < 0) {
        start += len;
        CLAMP_MIN(start, 0);
      }
      if (stop < 0) {
        stop += len;
        CLAMP_MIN(stop, 0);
      }
    }

    if (stop - start <= 0) {
      return PyList_New(0);
    }

    return pyprim_prop_collection_subscript_slice(self, start, stop);
  }
  if (PyTuple_Check(key)) {
    /* Special case, for ID datablocks we. */
    return pyprim_prop_collection_subscript_str_lib_pair(self,
                                                         key,
                                                         "kpy_prop_collection[id, lib]",
                                                         true);
  }

  PyErr_Format(PyExc_TypeError,
               "kpy_prop_collection[key]: invalid key, "
               "must be a string or an int, not %.200s",
               Py_TYPE(key)->tp_name);
  return NULL;
}

struct ItemConvertArgData;

typedef void (*ItemConvertFunc)(const struct ItemConvertArgData *arg, PyObject *, char *);
typedef int (*ItemTypeCheckFunc)(PyObject *);
typedef void (*PRIM_SetArrayFunc)(KrakenPRIM *, KrakenPROP *, const char *);
typedef void (*PRIM_SetIndexFunc)(KrakenPRIM *, KrakenPROP *, int index, void *);

struct ItemConvertArgData
{
  union
  {
    struct
    {
      int range[2];
    } int_data;
    struct
    {
      float range[2];
    } float_data;
  };
};

/**
 * Callback and args needed to apply the value (clamp range for now)
 */
typedef struct ItemConvert_FuncArg
{
  ItemConvertFunc func;
  struct ItemConvertArgData arg;
} ItemConvert_FuncArg;

/*
 * arr[3][4][5]
 *     0  1  2  <- dimension index
 */

/*
 *  arr[2] = x
 *
 *  py_to_array_index(arraydim=0, arrayoffset=0, index=2)
 *      validate_array(lvalue_dim=0)
 *      ... make real index ...
 */

/* arr[3] = x, self->arraydim is 0, lvalue_dim is 1 */

/**
 * @note
 * Ensures that a python sequence has expected number of
 * items/sub-items and items are of desired type. */
static int validate_array_type(PyObject *seq,
                               int dim,
                               int totdim,
                               int dimsize[],
                               const bool is_dynamic,
                               ItemTypeCheckFunc check_item_type,
                               const char *item_type_str,
                               const char *error_prefix)
{
  Py_ssize_t i;

  /* not the last dimension */
  if (dim + 1 < totdim) {
    /* check that a sequence contains dimsize[dim] items */
    const int seq_size = PySequence_Size(seq);
    if (seq_size == -1) {
      PyErr_Format(PyExc_ValueError,
                   "%s sequence expected at dimension %d, not '%s'",
                   error_prefix,
                   dim + 1,
                   Py_TYPE(seq)->tp_name);
      return -1;
    }
    for (i = 0; i < seq_size; i++) {
      Py_ssize_t item_seq_size;
      PyObject *item;
      bool ok = true;
      item = PySequence_GetItem(seq, i);

      if (item == NULL) {
        PyErr_Format(PyExc_TypeError,
                     "%s sequence type '%s' failed to retrieve index %d",
                     error_prefix,
                     Py_TYPE(seq)->tp_name,
                     i);
        ok = 0;
      } else if ((item_seq_size = PySequence_Size(item)) == -1) {
        // KLI_snprintf(error_str, error_str_size, "expected a sequence of %s", item_type_str);
        PyErr_Format(PyExc_TypeError,
                     "%s expected a sequence of %s, not %s",
                     error_prefix,
                     item_type_str,
                     Py_TYPE(item)->tp_name);
        ok = 0;
      }
      /* arr[3][4][5]
       * dimsize[1] = 4
       * dimsize[2] = 5
       *
       * dim = 0 */
      else if (item_seq_size != dimsize[dim + 1]) {
        /* KLI_snprintf(error_str, error_str_size,
         *              "sequences of dimension %d should contain %d items",
         *              dim + 1, dimsize[dim + 1]); */
        PyErr_Format(PyExc_ValueError,
                     "%s sequences of dimension %d should contain %d items, not %d",
                     error_prefix,
                     dim + 1,
                     dimsize[dim + 1],
                     item_seq_size);
        ok = 0;
      } else if (validate_array_type(item,
                                     dim + 1,
                                     totdim,
                                     dimsize,
                                     is_dynamic,
                                     check_item_type,
                                     item_type_str,
                                     error_prefix) == -1) {
        ok = 0;
      }

      Py_XDECREF(item);

      if (!ok) {
        return -1;
      }
    }
  } else {
    /* check that items are of correct type */
    const int seq_size = PySequence_Size(seq);
    if (seq_size == -1) {
      PyErr_Format(PyExc_ValueError,
                   "%s sequence expected at dimension %d, not '%s'",
                   error_prefix,
                   dim + 1,
                   Py_TYPE(seq)->tp_name);
      return -1;
    }
    if ((seq_size != dimsize[dim]) && (is_dynamic == false)) {
      PyErr_Format(PyExc_ValueError,
                   "%s sequences of dimension %d should contain %d items, not %d",
                   error_prefix,
                   dim,
                   dimsize[dim],
                   seq_size);
      return -1;
    }

    for (i = 0; i < seq_size; i++) {
      PyObject *item = PySequence_GetItem(seq, i);

      if (item == NULL) {
        PyErr_Format(PyExc_TypeError,
                     "%s sequence type '%s' failed to retrieve index %d",
                     error_prefix,
                     Py_TYPE(seq)->tp_name,
                     i);
        return -1;
      }
      if (!check_item_type(item)) {
        Py_DECREF(item);

#if 0
        KLI_snprintf(
            error_str, error_str_size, "sequence items should be of type %s", item_type_str);
#endif
        PyErr_Format(PyExc_TypeError,
                     "%s expected sequence items of type %s, not %s",
                     error_prefix,
                     item_type_str,
                     Py_TYPE(item)->tp_name);
        return -1;
      }

      Py_DECREF(item);
    }
  }

  return 0; /* ok */
}

/* Returns the number of items in a single- or multi-dimensional sequence. */
static int count_items(PyObject *seq, int dim)
{
  int totitem = 0;

  if (dim > 1) {
    const Py_ssize_t seq_size = PySequence_Size(seq);
    Py_ssize_t i;
    for (i = 0; i < seq_size; i++) {
      PyObject *item = PySequence_GetItem(seq, i);
      if (item) {
        const int tot = count_items(item, dim - 1);
        Py_DECREF(item);
        if (tot != -1) {
          totitem += tot;
        } else {
          totitem = -1;
          break;
        }
      } else {
        totitem = -1;
        break;
      }
    }
  } else {
    totitem = PySequence_Size(seq);
  }

  return totitem;
}

/* Modifies property array length if needed and PROP_DYNAMIC flag is set. */
static int validate_array_length(PyObject *rvalue,
                                 KrakenPRIM *ptr,
                                 KrakenPROP *prop,
                                 int lvalue_dim,
                                 int *r_totitem,
                                 const char *error_prefix)
{
  int dimsize[MAX_ARRAY_DIMENSION];
  int tot, totdim, len;

  totdim = LUXO_prop_array_dimension(ptr, prop, dimsize);
  tot = count_items(rvalue, totdim - lvalue_dim);

  if (tot == -1) {
    PyErr_Format(PyExc_ValueError,
                 "%s %.200s.%.200s, error validating the sequence length",
                 error_prefix,
                 LUXO_prim_identifier(ptr->type).data(),
                 LUXO_prop_identifier(prop).data());
    return -1;
  }
  if ((LUXO_prop_flag(prop) & PROP_DYNAMIC) && lvalue_dim == 0) {
    if (LUXO_prop_array_length(ptr, prop) != tot) {
#if 0
      /* length is flexible */
      if (!LUXO_prop_dynamic_array_set_length(ptr, prop, tot)) {
        /* KLI_snprintf(error_str, error_str_size,
         *              "%s.%s: array length cannot be changed to %d",
         *              LUXO_prim_identifier(ptr->type), LUXO_prop_identifier(prop), tot); */
        PyErr_Format(PyExc_ValueError,
                     "%s %s.%s: array length cannot be changed to %d",
                     error_prefix,
                     LUXO_prim_identifier(ptr->type).data(),
                     LUXO_prop_identifier(prop).data(),
                     tot);
        return -1;
      }
#else
      *r_totitem = tot;
      return 0;

#endif
    }

    len = tot;
  } else {
    /* length is a constraint */
    if (!lvalue_dim) {
      len = LUXO_prop_array_length(ptr, prop);
    }
    /* array item assignment */
    else {
      int i;

      len = 1;

      /* arr[3][4][5]
       *
       *    arr[2] = x
       *    dimsize = {4, 5}
       *    dimsize[1] = 4
       *    dimsize[2] = 5
       *    lvalue_dim = 0, totdim = 3
       *
       *    arr[2][3] = x
       *    lvalue_dim = 1
       *
       *    arr[2][3][4] = x
       *    lvalue_dim = 2 */
      for (i = lvalue_dim; i < totdim; i++) {
        len *= dimsize[i];
      }
    }

    if (tot != len) {
      // KLI_snprintf(error_str, error_str_size, "sequence must have length of %d", len);
      PyErr_Format(PyExc_ValueError,
                   "%s %.200s.%.200s, sequence must have %d items total, not %d",
                   error_prefix,
                   LUXO_prim_identifier(ptr->type).data(),
                   LUXO_prop_identifier(prop).data(),
                   len,
                   tot);
      return -1;
    }
  }

  *r_totitem = len;

  return 0;
}

static int validate_array(PyObject *rvalue,
                          KrakenPRIM *ptr,
                          KrakenPROP *prop,
                          int lvalue_dim,
                          ItemTypeCheckFunc check_item_type,
                          const char *item_type_str,
                          int *r_totitem,
                          const char *error_prefix)
{
  int dimsize[MAX_ARRAY_DIMENSION];
  const int totdim = LUXO_prop_array_dimension(ptr, prop, dimsize);

  /* validate type first because length validation may modify property array length */

#ifdef USE_MATHUTILS
  if (lvalue_dim == 0) { /* only valid for first level array */
    if (MatrixObject_Check(rvalue)) {
      MatrixObject *pymat = (MatrixObject *)rvalue;

      if (BaseMath_ReadCallback(pymat) == -1) {
        return -1;
      }

      if (LUXO_prop_type(prop) != PROP_FLOAT) {
        PyErr_Format(PyExc_ValueError,
                     "%s %.200s.%.200s, matrix assign to non float array",
                     error_prefix,
                     LUXO_prim_identifier(ptr->type).data(),
                     LUXO_prop_identifier(prop).data());
        return -1;
      }
      if (totdim != 2) {
        PyErr_Format(PyExc_ValueError,
                     "%s %.200s.%.200s, matrix assign array with %d dimensions",
                     error_prefix,
                     LUXO_prim_identifier(ptr->type).data(),
                     LUXO_prop_identifier(prop).data(),
                     totdim);
        return -1;
      }
      if (pymat->col_num != dimsize[0] || pymat->row_num != dimsize[1]) {
        PyErr_Format(PyExc_ValueError,
                     "%s %.200s.%.200s, matrix assign dimension size mismatch, "
                     "is %dx%d, expected be %dx%d",
                     error_prefix,
                     LUXO_prim_identifier(ptr->type).data(),
                     LUXO_prop_identifier(prop).data(),
                     pymat->col_num,
                     pymat->row_num,
                     dimsize[0],
                     dimsize[1]);
        return -1;
      }

      *r_totitem = dimsize[0] * dimsize[1];
      return 0;
    }
  }
#endif /* USE_MATHUTILS */

  {
    const int prop_flag = LUXO_prop_flag(prop);
    if (validate_array_type(rvalue,
                            lvalue_dim,
                            totdim,
                            dimsize,
                            (prop_flag & PROP_DYNAMIC) != 0,
                            check_item_type,
                            item_type_str,
                            error_prefix) == -1) {
      return -1;
    }

    return validate_array_length(rvalue, ptr, prop, lvalue_dim, r_totitem, error_prefix);
  }
}

static char *copy_value_single(PyObject *item,
                               KrakenPRIM *ptr,
                               KrakenPROP *prop,
                               char *data,
                               uint item_size,
                               int *index,
                               const ItemConvert_FuncArg *convert_item,
                               PRIM_SetIndexFunc rna_set_index)
{
  if (!data) {
    union
    {
      float fl;
      int i;
    } value_buf;
    char *value = static_cast<char *>((void *)&value_buf);

    convert_item->func(&convert_item->arg, item, value);
    rna_set_index(ptr, prop, *index, value);
    (*index) += 1;
  } else {
    convert_item->func(&convert_item->arg, item, data);
    data += item_size;
  }

  return data;
}

static char *copy_values(PyObject *seq,
                         KrakenPRIM *ptr,
                         KrakenPROP *prop,
                         int dim,
                         char *data,
                         uint item_size,
                         int *index,
                         const ItemConvert_FuncArg *convert_item,
                         PRIM_SetIndexFunc rna_set_index)
{
  const int totdim = LUXO_prop_array_dimension(ptr, prop, NULL);
  const Py_ssize_t seq_size = PySequence_Size(seq);
  Py_ssize_t i;

  /* Regarding PySequence_GetItem() failing.
   *
   * This should never be NULL since we validated it, _but_ some tricky python
   * developer could write their own sequence type which succeeds on
   * validating but fails later somehow, so include checks for safety.
   */

  /* Note that 'data can be NULL' */

  if (seq_size == -1) {
    return NULL;
  }

#ifdef USE_MATHUTILS
  if (dim == 0) {
    if (MatrixObject_Check(seq)) {
      MatrixObject *pymat = (MatrixObject *)seq;
      const size_t allocsize = pymat->col_num * pymat->row_num * sizeof(float);

      /* read callback already done by validate */
      /* since this is the first iteration we can assume data is allocated */
      memcpy(data, pymat->matrix, allocsize);

      /* not really needed but do for completeness */
      data += allocsize;

      return data;
    }
  }
#endif /* USE_MATHUTILS */

  for (i = 0; i < seq_size; i++) {
    PyObject *item = PySequence_GetItem(seq, i);
    if (item) {
      if (dim + 1 < totdim) {
        data = copy_values(item,
                           ptr,
                           prop,
                           dim + 1,
                           data,
                           item_size,
                           index,
                           convert_item,
                           rna_set_index);
      } else {
        data =
          copy_value_single(item, ptr, prop, data, item_size, index, convert_item, rna_set_index);
      }

      Py_DECREF(item);

      /* data may be NULL, but the for loop checks */
    } else {
      return NULL;
    }
  }

  return data;
}

static int py_to_array_index(PyObject *py,
                             KrakenPRIM *ptr,
                             KrakenPROP *prop,
                             int lvalue_dim,
                             int arrayoffset,
                             int index,
                             ItemTypeCheckFunc check_item_type,
                             const char *item_type_str,
                             const ItemConvert_FuncArg *convert_item,
                             PRIM_SetIndexFunc rna_set_index,
                             const char *error_prefix)
{
  int totdim, dimsize[MAX_ARRAY_DIMENSION];
  int totitem, i;

  totdim = LUXO_prop_array_dimension(ptr, prop, dimsize);

  /* convert index */

  /* arr[3][4][5]
   *
   *    arr[2] = x
   *    lvalue_dim = 0, index = 0 + 2 * 4 * 5
   *
   *    arr[2][3] = x
   *    lvalue_dim = 1, index = 40 + 3 * 5 */

  lvalue_dim++;

  for (i = lvalue_dim; i < totdim; i++) {
    index *= dimsize[i];
  }

  index += arrayoffset;

  if (lvalue_dim == totdim) { /* single item, assign directly */
    if (!check_item_type(py)) {
      PyErr_Format(PyExc_TypeError,
                   "%s %.200s.%.200s, expected a %s type, not %s",
                   error_prefix,
                   LUXO_prim_identifier(ptr->type).data(),
                   LUXO_prop_identifier(prop).data(),
                   item_type_str,
                   Py_TYPE(py)->tp_name);
      return -1;
    }
    copy_value_single(py, ptr, prop, NULL, 0, &index, convert_item, rna_set_index);
  } else {
    if (validate_array(py,
                       ptr,
                       prop,
                       lvalue_dim,
                       check_item_type,
                       item_type_str,
                       &totitem,
                       error_prefix) == -1) {
      return -1;
    }

    if (totitem) {
      copy_values(py, ptr, prop, lvalue_dim, NULL, 0, &index, convert_item, rna_set_index);
    }
  }
  return 0;
}

static void py_to_float(const struct ItemConvertArgData *arg, PyObject *py, char *data)
{
  const float *range = arg->float_data.range;
  float value = (float)PyFloat_AsDouble(py);
  CLAMP(value, range[0], range[1]);
  *(float *)data = value;
}

static void py_to_int(const struct ItemConvertArgData *arg, PyObject *py, char *data)
{
  const int *range = arg->int_data.range;
  int value = PyC_Long_AsI32(py);
  CLAMP(value, range[0], range[1]);
  *(int *)data = value;
}

static void py_to_bool(const struct ItemConvertArgData *UNUSED(arg), PyObject *py, char *data)
{
  *(bool *)data = (bool)PyObject_IsTrue(py);
}

static int py_float_check(PyObject *py)
{
  /* accept both floats and integers */
  return PyNumber_Check(py);
}

static int py_int_check(PyObject *py)
{
  /* accept only integers */
  return PyLong_Check(py);
}

static int py_bool_check(PyObject *py)
{
  return PyBool_Check(py);
}

static void float_set_index(KrakenPRIM *ptr, KrakenPROP *prop, int index, void *value)
{
  LUXO_prop_float_set_index(ptr, prop, index, *(float *)value);
}

static void int_set_index(KrakenPRIM *ptr, KrakenPROP *prop, int index, void *value)
{
  LUXO_prop_int_set_index(ptr, prop, index, *(int *)value);
}

static void bool_set_index(KrakenPRIM *ptr, KrakenPROP *prop, int index, void *value)
{
  LUXO_prop_boolean_set_index(ptr, prop, index, *(bool *)value);
}

static void convert_item_init_float(KrakenPRIM *ptr,
                                    KrakenPROP *prop,
                                    ItemConvert_FuncArg *convert_item)
{
  float *range = convert_item->arg.float_data.range;
  convert_item->func = py_to_float;
  LUXO_prop_float_range(ptr, prop, &range[0], &range[1]);
}

static void convert_item_init_int(KrakenPRIM *ptr,
                                  KrakenPROP *prop,
                                  ItemConvert_FuncArg *convert_item)
{
  int *range = convert_item->arg.int_data.range;
  convert_item->func = py_to_int;
  LUXO_prop_int_range(ptr, prop, &range[0], &range[1]);
}

static void convert_item_init_bool(KrakenPRIM *UNUSED(ptr),
                                   KrakenPROP *UNUSED(prop),
                                   ItemConvert_FuncArg *convert_item)
{
  convert_item->func = py_to_bool;
}


static int pyprim_py_to_array_index(KrakenPRIM *ptr,
                                    KrakenPROP *prop,
                                    int arraydim,
                                    int arrayoffset,
                                    int index,
                                    PyObject *py,
                                    const char *error_prefix)
{
  int ret;
  switch (LUXO_prop_type(prop)) {
    case PROP_FLOAT: {
      ItemConvert_FuncArg convert_item;
      convert_item_init_float(ptr, prop, &convert_item);

      ret = py_to_array_index(py,
                              ptr,
                              prop,
                              arraydim,
                              arrayoffset,
                              index,
                              py_float_check,
                              "float",
                              &convert_item,
                              float_set_index,
                              error_prefix);
      break;
    }
    case PROP_INT: {
      ItemConvert_FuncArg convert_item;
      convert_item_init_int(ptr, prop, &convert_item);

      ret = py_to_array_index(py,
                              ptr,
                              prop,
                              arraydim,
                              arrayoffset,
                              index,
                              py_int_check,
                              "int",
                              &convert_item,
                              int_set_index,
                              error_prefix);
      break;
    }
    case PROP_BOOLEAN: {
      ItemConvert_FuncArg convert_item;
      convert_item_init_bool(ptr, prop, &convert_item);

      ret = py_to_array_index(py,
                              ptr,
                              prop,
                              arraydim,
                              arrayoffset,
                              index,
                              py_bool_check,
                              "boolean",
                              &convert_item,
                              bool_set_index,
                              error_prefix);
      break;
    }
    default: {
      PyErr_SetString(PyExc_TypeError, "not an array type");
      ret = -1;
      break;
    }
  }

  return ret;
}

static int pyprim_py_to_prop_array_index(KPy_StagePropARRAY *self, int index, PyObject *value)
{
  int ret = 0;
  KrakenPRIM *ptr = &self->ptr;
  KrakenPROP *prop = self->prop;

  const int totdim = LUXO_prop_array_dimension(ptr, prop, NULL);

  if (totdim > 1) {
    // char error_str[512];
    if (pyprim_py_to_array_index(&self->ptr,
                                 self->prop,
                                 self->arraydim,
                                 self->arrayoffset,
                                 index,
                                 value,
                                 "") == -1) {
      /* Error is set. */
      ret = -1;
    }
  } else {
    /* See if we can coerce into a Python type - 'PropertyType'. */
    switch (LUXO_prop_type(prop)) {
      case PROP_BOOLEAN: {
        const int param = PyC_Long_AsBool(value);

        if (param == -1) {
          /* Error is set. */
          ret = -1;
        } else {
          LUXO_prop_boolean_set_index(ptr, prop, index, param);
        }
        break;
      }
      case PROP_INT: {
        int param = PyC_Long_AsI32(value);
        if (param == -1 && PyErr_Occurred()) {
          PyErr_SetString(PyExc_TypeError, "expected an int type");
          ret = -1;
        } else {
          LUXO_prop_int_clamp(ptr, prop, &param);
          LUXO_prop_int_set_index(ptr, prop, index, param);
        }
        break;
      }
      case PROP_FLOAT: {
        float param = PyFloat_AsDouble(value);
        if (PyErr_Occurred()) {
          PyErr_SetString(PyExc_TypeError, "expected a float type");
          ret = -1;
        } else {
          LUXO_prop_float_clamp(ptr, prop, &param);
          LUXO_prop_float_set_index(ptr, prop, index, param);
        }
        break;
      }
      default:
        PyErr_SetString(PyExc_AttributeError, "not an array type");
        ret = -1;
        break;
    }
  }

  /* Run RNA property functions. */
  if (LUXO_prop_update_check(prop)) {
    LUXO_prop_update(KPY_context_get(), ptr, prop);
  }

  return ret;
}

/* generic check to see if a PyObject is compatible with a collection
 * -1 on failure, 0 on success, sets the error */
static int pyprim_prop_collection_type_check(KPy_StagePROP *self, PyObject *value)
{
  KrakenPRIM *prop_sprim;

  if (value == Py_None) {
    if (LUXO_prop_flag(self->prop) & PROP_NEVER_NULL) {
      PyErr_Format(PyExc_TypeError,
                   "kpy_prop_collection[key] = value: invalid, "
                   "this collection doesn't support None assignment");
      return -1;
    }

    return 0; /* None is OK. */
  }
  if (KPy_StagePRIM_Check(value) == 0) {
    PyErr_Format(PyExc_TypeError,
                 "kpy_prop_collection[key] = value: invalid, "
                 "expected a StructRNA type or None, not a %.200s",
                 Py_TYPE(value)->tp_name);
    return -1;
  }
  if ((prop_sprim = LUXO_prop_pointer_type(&self->ptr, self->prop))) {
    KrakenPRIM *value_sprim = ((KPy_StagePRIM *)value)->ptr.type;
    if (LUXO_prim_is_a(value_sprim, prop_sprim) == 0) {
      PyErr_Format(PyExc_TypeError,
                   "kpy_prop_collection[key] = value: invalid, "
                   "expected a '%.200s' type or None, not a '%.200s'",
                   LUXO_prim_identifier(prop_sprim).data(),
                   LUXO_prim_identifier(value_sprim).data());
      return -1;
    }

    return 0; /* OK, this is the correct type! */
  }

  PyErr_Format(PyExc_TypeError,
               "kpy_prop_collection[key] = value: internal error, "
               "failed to get the collection type");
  return -1;
}

/* NOTE: currently this is a copy of 'pyprim_prop_collection_subscript' with
 * large blocks commented, we may support slice/key indices later */
static int pyprim_prop_collection_ass_subscript(KPy_StagePROP *self,
                                                PyObject *key,
                                                PyObject *value)
{
  PYSTAGE_PROP_CHECK_INT(self);

  /* Validate the assigned value. */
  if (value == NULL) {
    PyErr_SetString(PyExc_TypeError, "del kpy_prop_collection[key]: not supported");
    return -1;
  }
  if (pyprim_prop_collection_type_check(self, value) == -1) {
    return -1; /* Exception is set. */
  }

#if 0
  if (PyUnicode_Check(key)) {
    return pyprim_prop_collection_subscript_str(self, PyUnicode_AsUTF8(key));
  }
  else
#endif
  if (PyIndex_Check(key)) {
    const Py_ssize_t i = PyNumber_AsSsize_t(key, PyExc_IndexError);
    if (i == -1 && PyErr_Occurred()) {
      return -1;
    }

    return pyprim_prop_collection_ass_subscript_int(self, i, value);
  }
#if 0 /* TODO: fake slice assignment. */
  else if (PySlice_Check(key)) {
    PySliceObject *key_slice = (PySliceObject *)key;
    Py_ssize_t step = 1;

    if (key_slice->step != Py_None && !_PyEval_SliceIndex(key, &step)) {
      return NULL;
    }
    else if (step != 1) {
      PyErr_SetString(PyExc_TypeError, "kpy_prop_collection[slice]: slice steps not supported");
      return NULL;
    }
    else if (key_slice->start == Py_None && key_slice->stop == Py_None) {
      return pyprim_prop_collection_subscript_slice(self, 0, PY_SSIZE_T_MAX);
    }
    else {
      Py_ssize_t start = 0, stop = PY_SSIZE_T_MAX;

      /* Avoid PySlice_GetIndicesEx because it needs to know the length ahead of time. */
      if (key_slice->start != Py_None && !_PyEval_SliceIndex(key_slice->start, &start)) {
        return NULL;
      }
      if (key_slice->stop != Py_None && !_PyEval_SliceIndex(key_slice->stop, &stop)) {
        return NULL;
      }

      if (start < 0 || stop < 0) {
        /* Only get the length for negative values. */
        Py_ssize_t len = (Py_ssize_t)LUXO_prop_collection_length(&self->ptr, self->prop);
        if (start < 0) {
          start += len;
          CLAMP_MIN(start, 0);
        }
        if (stop < 0) {
          stop += len;
          CLAMP_MIN(stop, 0);
        }
      }

      if (stop - start <= 0) {
        return PyList_New(0);
      }
      else {
        return pyprim_prop_collection_subscript_slice(self, start, stop);
      }
    }
  }
#endif

  PyErr_Format(PyExc_TypeError,
               "kpy_prop_collection[key]: invalid key, "
               "must be a string or an int, not %.200s",
               Py_TYPE(key)->tp_name);
  return -1;
}

/**
 * Helpers for #prop_subscript_ass_array_slice
 */

static PyObject *prop_subscript_ass_array_slice__as_seq_fast(PyObject *value, int length)
{
  PyObject *value_fast;
  if (!(value_fast = PySequence_Fast(value,
                                     "kpy_prop_array[slice] = value: "
                                     "element in assignment is not a sequence type"))) {
    return NULL;
  }
  if (PySequence_Fast_GET_SIZE(value_fast) != length) {
    Py_DECREF(value_fast);
    PyErr_SetString(PyExc_ValueError,
                    "kpy_prop_array[slice] = value: "
                    "re-sizing kpy_struct element in arrays isn't supported");

    return NULL;
  }

  return value_fast;
}

static int prop_subscript_ass_array_slice__float_recursive(PyObject **value_items,
                                                           float *value,
                                                           int totdim,
                                                           const int dimsize[],
                                                           const float range[2])
{
  const int length = dimsize[0];
  if (totdim > 1) {
    int index = 0;
    int i;
    for (i = 0; i != length; i++) {
      PyObject *subvalue = prop_subscript_ass_array_slice__as_seq_fast(value_items[i], dimsize[1]);
      if (UNLIKELY(subvalue == NULL)) {
        return 0;
      }

      index += prop_subscript_ass_array_slice__float_recursive(PySequence_Fast_ITEMS(subvalue),
                                                               &value[index],
                                                               totdim - 1,
                                                               &dimsize[1],
                                                               range);

      Py_DECREF(subvalue);
    }
    return index;
  }

  KLI_assert(totdim == 1);
  const float min = range[0], max = range[1];
  int i;
  for (i = 0; i != length; i++) {
    float v = PyFloat_AsDouble(value_items[i]);
    CLAMP(v, min, max);
    value[i] = v;
  }
  return i;
}

static int prop_subscript_ass_array_slice__int_recursive(PyObject **value_items,
                                                         int *value,
                                                         int totdim,
                                                         const int dimsize[],
                                                         const int range[2])
{
  const int length = dimsize[0];
  if (totdim > 1) {
    int index = 0;
    int i;
    for (i = 0; i != length; i++) {
      PyObject *subvalue = prop_subscript_ass_array_slice__as_seq_fast(value_items[i], dimsize[1]);
      if (UNLIKELY(subvalue == NULL)) {
        return 0;
      }

      index += prop_subscript_ass_array_slice__int_recursive(PySequence_Fast_ITEMS(subvalue),
                                                             &value[index],
                                                             totdim - 1,
                                                             &dimsize[1],
                                                             range);

      Py_DECREF(subvalue);
    }
    return index;
  }

  KLI_assert(totdim == 1);
  const int min = range[0], max = range[1];
  int i;
  for (i = 0; i != length; i++) {
    int v = PyLong_AsLong(value_items[i]);
    CLAMP(v, min, max);
    value[i] = v;
  }
  return i;
}

static int prop_subscript_ass_array_slice__bool_recursive(PyObject **value_items,
                                                          bool *value,
                                                          int totdim,
                                                          const int dimsize[])
{
  const int length = dimsize[0];
  if (totdim > 1) {
    int index = 0;
    int i;
    for (i = 0; i != length; i++) {
      PyObject *subvalue = prop_subscript_ass_array_slice__as_seq_fast(value_items[i], dimsize[1]);
      if (UNLIKELY(subvalue == NULL)) {
        return 0;
      }

      index += prop_subscript_ass_array_slice__bool_recursive(PySequence_Fast_ITEMS(subvalue),
                                                              &value[index],
                                                              totdim - 1,
                                                              &dimsize[1]);

      Py_DECREF(subvalue);
    }
    return index;
  }

  KLI_assert(totdim == 1);
  int i;
  for (i = 0; i != length; i++) {
    const int v = PyLong_AsLong(value_items[i]);
    value[i] = v;
  }
  return i;
}

/* Could call `pyprim_py_to_prop_array_index(self, i, value)` in a loop, but it is slow. */
static int prop_subscript_ass_array_slice(KrakenPRIM *ptr,
                                          KrakenPROP *prop,
                                          int arraydim,
                                          int arrayoffset,
                                          int start,
                                          int stop,
                                          int length,
                                          PyObject *value_orig)
{
  const int length_flat = LUXO_prop_array_length(ptr, prop);
  PyObject *value;
  void *values_alloc = NULL;
  int ret = 0;

  if (value_orig == NULL) {
    PyErr_SetString(
      PyExc_TypeError,
      "kpy_prop_array[slice] = value: deleting with list types is not supported by kpy_struct");
    return -1;
  }

  if (!(value = PySequence_Fast(
          value_orig,
          "kpy_prop_array[slice] = value: assignment is not a sequence type"))) {
    return -1;
  }

  if (PySequence_Fast_GET_SIZE(value) != stop - start) {
    Py_DECREF(value);
    PyErr_SetString(PyExc_TypeError,
                    "kpy_prop_array[slice] = value: re-sizing kpy_struct arrays isn't supported");
    return -1;
  }

  int dimsize[3];
  const int totdim = LUXO_prop_array_dimension(ptr, prop, dimsize);
  if (totdim > 1) {
    KLI_assert(dimsize[arraydim] == length);
  }

  int span = 1;
  if (totdim > 1) {
    for (int i = arraydim + 1; i < totdim; i++) {
      span *= dimsize[i];
    }
  }

  PyObject **value_items = PySequence_Fast_ITEMS(value);
  switch (LUXO_prop_type(prop)) {
    case PROP_FLOAT: {
      float values_stack[PYPRIM_STACK_ARRAY];
      float *values = (length_flat > PYPRIM_STACK_ARRAY) ?
                        (float *)(values_alloc = PyMem_MALLOC(sizeof(*values) * length_flat)) :
                        values_stack;
      if (start != 0 || stop != length) {
        /* Partial assignment? - need to get the array. */
        LUXO_prop_float_get_array(ptr, prop, values);
      }

      float range[2];
      LUXO_prop_float_range(ptr, prop, &range[0], &range[1]);

      dimsize[arraydim] = stop - start;
      prop_subscript_ass_array_slice__float_recursive(value_items,
                                                      &values[arrayoffset + (start * span)],
                                                      totdim - arraydim,
                                                      &dimsize[arraydim],
                                                      range);

      if (PyErr_Occurred()) {
        ret = -1;
      } else {
        LUXO_prop_float_set_array(ptr, prop, values);
      }
      break;
    }
    case PROP_INT: {
      int values_stack[PYPRIM_STACK_ARRAY];
      int *values = (length_flat > PYPRIM_STACK_ARRAY) ?
                      (int *)(values_alloc = PyMem_MALLOC(sizeof(*values) * length_flat)) :
                      values_stack;
      if (start != 0 || stop != length) {
        /* Partial assignment? - need to get the array. */
        LUXO_prop_int_get_array(ptr, prop, values);
      }

      int range[2];
      LUXO_prop_int_range(ptr, prop, &range[0], &range[1]);

      dimsize[arraydim] = stop - start;
      prop_subscript_ass_array_slice__int_recursive(value_items,
                                                    &values[arrayoffset + (start * span)],
                                                    totdim - arraydim,
                                                    &dimsize[arraydim],
                                                    range);

      if (PyErr_Occurred()) {
        ret = -1;
      } else {
        LUXO_prop_int_set_array(ptr, prop, values);
      }
      break;
    }
    case PROP_BOOLEAN: {
      bool values_stack[PYPRIM_STACK_ARRAY];
      bool *values = (length_flat > PYPRIM_STACK_ARRAY) ?
                       (bool *)(values_alloc = PyMem_MALLOC(sizeof(bool) * length_flat)) :
                       values_stack;

      if (start != 0 || stop != length) {
        /* Partial assignment? - need to get the array. */
        LUXO_prop_boolean_get_array(ptr, prop, values);
      }

      dimsize[arraydim] = stop - start;
      prop_subscript_ass_array_slice__bool_recursive(value_items,
                                                     &values[arrayoffset + (start * span)],
                                                     totdim - arraydim,
                                                     &dimsize[arraydim]);

      if (PyErr_Occurred()) {
        ret = -1;
      } else {
        LUXO_prop_boolean_set_array(ptr, prop, values);
      }
      break;
    }
    default:
      PyErr_SetString(PyExc_TypeError, "not an array type");
      ret = -1;
      break;
  }

  Py_DECREF(value);

  if (values_alloc) {
    PyMem_FREE(values_alloc);
  }

  return ret;
}

static int prop_subscript_ass_array_int(KPy_StagePropARRAY *self,
                                        Py_ssize_t keynum,
                                        PyObject *value)
{
  PYSTAGE_PROP_CHECK_INT((KPy_StagePROP *)self);

  int len = pyprim_prop_array_length(self);

  if (keynum < 0) {
    keynum += len;
  }

  if (keynum >= 0 && keynum < len) {
    return pyprim_py_to_prop_array_index(self, keynum, value);
  }

  PyErr_SetString(PyExc_IndexError, "kpy_prop_array[index] = value: index out of range");
  return -1;
}

static int pyprim_prop_array_ass_subscript(KPy_StagePropARRAY *self,
                                           PyObject *key,
                                           PyObject *value)
{
  // char *keyname = NULL; /* Not supported yet. */
  int ret = -1;

  PYSTAGE_PROP_CHECK_INT((KPy_StagePROP *)self);

  if (!LUXO_prop_editable_flag(&self->ptr, self->prop)) {
    PyErr_Format(PyExc_AttributeError,
                 "kpy_prop_collection: attribute \"%.200s\" from \"%.200s\" is read-only",
                 LUXO_prop_identifier(self->prop).data(),
                 LUXO_prim_identifier(self->ptr.type).data());
    ret = -1;
  }

  else if (PyIndex_Check(key)) {
    const Py_ssize_t i = PyNumber_AsSsize_t(key, PyExc_IndexError);
    if (i == -1 && PyErr_Occurred()) {
      ret = -1;
    } else {
      ret = prop_subscript_ass_array_int(self, i, value);
    }
  } else if (PySlice_Check(key)) {
    const Py_ssize_t len = pyprim_prop_array_length(self);
    Py_ssize_t start, stop, step, slicelength;

    if (PySlice_GetIndicesEx(key, len, &start, &stop, &step, &slicelength) < 0) {
      ret = -1;
    } else if (slicelength <= 0) {
      ret = 0; /* Do nothing. */
    } else if (step == 1) {
      ret = prop_subscript_ass_array_slice(&self->ptr,
                                           self->prop,
                                           self->arraydim,
                                           self->arrayoffset,
                                           start,
                                           stop,
                                           len,
                                           value);
    } else {
      PyErr_SetString(PyExc_TypeError, "slice steps not supported with RNA");
      ret = -1;
    }
  } else {
    PyErr_SetString(PyExc_AttributeError, "invalid key, key must be an int");
    ret = -1;
  }

  if (ret != -1) {
    if (LUXO_prop_update_check(self->prop)) {
      LUXO_prop_update(KPY_context_get(), &self->ptr, self->prop);
    }
  }

  return ret;
}



/* ---------------sequence------------------------------------------- */
Py_ssize_t pyprim_prop_array_length(KPy_StagePropARRAY *self)
{
  PYSTAGE_PROP_CHECK_INT((KPy_StagePROP *)self);

  if (LUXO_prop_array_dimension(&self->ptr, self->prop, NULL) > 1) {
    return LUXO_prop_multi_array_length(&self->ptr, self->prop, self->arraydim);
  }

  return LUXO_prop_array_length(&self->ptr, self->prop);
}

Py_ssize_t pyprim_prop_collection_length(KPy_StagePROP *self)
{
  PYSTAGE_PROP_CHECK_INT(self);

  return LUXO_prop_collection_length(&self->ptr, self->prop);
}



/* For slice only. */
static PyMappingMethods pyprim_prop_array_as_mapping = {
  (lenfunc)pyprim_prop_array_length,              /* mp_length */
  (binaryfunc)pyprim_prop_array_subscript,        /* mp_subscript */
  (objobjargproc)pyprim_prop_array_ass_subscript, /* mp_ass_subscript */
};

static PyMappingMethods pyprim_prop_collection_as_mapping = {
  (lenfunc)pyprim_prop_collection_length,              /* mp_length */
  (binaryfunc)pyprim_prop_collection_subscript,        /* mp_subscript */
  (objobjargproc)pyprim_prop_collection_ass_subscript, /* mp_ass_subscript */
};

/* Only for fast bool's, large structs, assign nb_bool on init. */
static PyNumberMethods pyprim_prop_array_as_number = {
  NULL,                            /* nb_add */
  NULL,                            /* nb_subtract */
  NULL,                            /* nb_multiply */
  NULL,                            /* nb_remainder */
  NULL,                            /* nb_divmod */
  NULL,                            /* nb_power */
  NULL,                            /* nb_negative */
  NULL,                            /* nb_positive */
  NULL,                            /* nb_absolute */
  (inquiry)pyprim_prop_array_bool, /* nb_bool */
};
static PyNumberMethods pyprim_prop_collection_as_number = {
  NULL,                                 /* nb_add */
  NULL,                                 /* nb_subtract */
  NULL,                                 /* nb_multiply */
  NULL,                                 /* nb_remainder */
  NULL,                                 /* nb_divmod */
  NULL,                                 /* nb_power */
  NULL,                                 /* nb_negative */
  NULL,                                 /* nb_positive */
  NULL,                                 /* nb_absolute */
  (inquiry)pyprim_prop_collection_bool, /* nb_bool */
};

static int pyprim_array_contains_py(KrakenPRIM *ptr, KrakenPROP *prop, PyObject *value)
{
  /* TODO: multi-dimensional arrays. */

  const int len = LUXO_prop_array_length(ptr, prop);
  int type;
  int i;

  if (len == 0) {
    /* possible with dynamic arrays */
    return 0;
  }

  if (LUXO_prop_array_dimension(ptr, prop, NULL) > 1) {
    PyErr_SetString(PyExc_TypeError, "PropertyRNA - multi dimensional arrays not supported yet");
    return -1;
  }

  type = LUXO_prop_type(prop);

  switch (type) {
    case PROP_FLOAT: {
      const float value_f = PyFloat_AsDouble(value);
      if (value_f == -1 && PyErr_Occurred()) {
        PyErr_Clear();
        return 0;
      }

      float tmp[32];
      float *tmp_arr;

      if (len * sizeof(float) > sizeof(tmp)) {
        tmp_arr = static_cast<float *>(PyMem_MALLOC(len * sizeof(float)));
      } else {
        tmp_arr = tmp;
      }

      LUXO_prop_float_get_array(ptr, prop, tmp_arr);

      for (i = 0; i < len; i++) {
        if (tmp_arr[i] == value_f) {
          break;
        }
      }

      if (tmp_arr != tmp) {
        PyMem_FREE(tmp_arr);
      }

      return i < len ? 1 : 0;

      break;
    }
    case PROP_INT: {
      const int value_i = PyC_Long_AsI32(value);
      if (value_i == -1 && PyErr_Occurred()) {
        PyErr_Clear();
        return 0;
      }

      int tmp[32];
      int *tmp_arr;

      if (len * sizeof(int) > sizeof(tmp)) {
        tmp_arr = static_cast<int *>(PyMem_MALLOC(len * sizeof(int)));
      } else {
        tmp_arr = tmp;
      }

      LUXO_prop_int_get_array(ptr, prop, tmp_arr);

      for (i = 0; i < len; i++) {
        if (tmp_arr[i] == value_i) {
          break;
        }
      }

      if (tmp_arr != tmp) {
        PyMem_FREE(tmp_arr);
      }

      return i < len ? 1 : 0;

      break;
    }
    case PROP_BOOLEAN: {
      const int value_i = PyC_Long_AsBool(value);
      if (value_i == -1 && PyErr_Occurred()) {
        PyErr_Clear();
        return 0;
      }

      bool tmp[32];
      bool *tmp_arr;

      if (len * sizeof(bool) > sizeof(tmp)) {
        tmp_arr = static_cast<bool *>(PyMem_MALLOC(len * sizeof(bool)));
      } else {
        tmp_arr = tmp;
      }

      LUXO_prop_boolean_get_array(ptr, prop, tmp_arr);

      for (i = 0; i < len; i++) {
        if (tmp_arr[i] == value_i) {
          break;
        }
      }

      if (tmp_arr != tmp) {
        PyMem_FREE(tmp_arr);
      }

      return i < len ? 1 : 0;

      break;
    }
  }

  /* should never reach this */
  PyErr_SetString(PyExc_TypeError, "PropertyRNA - type not in float/bool/int");
  return -1;
}

static int pyprim_prop_array_contains(KPy_StagePROP *self, PyObject *value)
{
  return pyprim_array_contains_py(&self->ptr, self->prop, value);
}

static int pyprim_prop_collection_contains(KPy_StagePROP *self, PyObject *key)
{
  KrakenPRIM newptr; /* Not used, just so LUXO_prop_collection_lookup_string runs. */

  if (PyTuple_Check(key)) {
    /* Special case, for ID data-blocks. */
    return pyprim_prop_collection_subscript_str_lib_pair_ptr(self,
                                                             key,
                                                             "(id, lib) in kpy_prop_collection",
                                                             false,
                                                             NULL);
  }

  /* Key in dict style check. */
  const char *keyname = PyUnicode_AsUTF8(key);

  if (keyname == NULL) {
    PyErr_SetString(PyExc_TypeError,
                    "kpy_prop_collection.__contains__: expected a string or a tuple of strings");
    return -1;
  }

  if (LUXO_prop_collection_lookup_string(&self->ptr, self->prop, keyname, &newptr)) {
    return 1;
  }

  return 0;
}

static int pyprim_prim_contains(KPy_StagePRIM *self, PyObject *value)
{
  const char *name = PyUnicode_AsUTF8(value);

  PYSTAGE_PRIM_CHECK_INT(self);

  if (!name) {
    PyErr_SetString(PyExc_TypeError, "kpy_struct.__contains__: expected a string");
    return -1;
  }

  if (LUXO_prim_idprops_check(self->ptr.type) == 0) {
    PyErr_SetString(PyExc_TypeError, "kpy_struct: this type doesn't support IDProperties");
    return -1;
  }

  IDProperty *group = LUXO_prim_idprops(&self->ptr, 0);

  if (!group) {
    return 0;
  }

  return IDP_GetPropertyFromGroup(group, TfToken(name)) ? 1 : 0;
}

static PySequenceMethods pyprim_prop_array_as_sequence = {
  (lenfunc)pyprim_prop_array_length,
  NULL, /* sq_concat */
  NULL, /* sq_repeat */
  (ssizeargfunc)pyprim_prop_array_subscript_int,
  /* sq_item */ /* Only set this so PySequence_Check() returns True */
  NULL,         /* sq_slice */
  (ssizeobjargproc)prop_subscript_ass_array_int, /* sq_ass_item */
  NULL,                                          /* *was* sq_ass_slice */
  (objobjproc)pyprim_prop_array_contains,        /* sq_contains */
  (binaryfunc)NULL,                              /* sq_inplace_concat */
  (ssizeargfunc)NULL,                            /* sq_inplace_repeat */
};

static PySequenceMethods pyprim_prop_collection_as_sequence = {
  (lenfunc)pyprim_prop_collection_length,
  NULL, /* sq_concat */
  NULL, /* sq_repeat */
  (ssizeargfunc)pyprim_prop_collection_subscript_int,
  /* sq_item */                         /* Only set this so PySequence_Check() returns True */
  NULL,                                 /* *was* sq_slice */
  (ssizeobjargproc)                     /* pyprim_prop_collection_ass_subscript_int */
  NULL /* let mapping take this one */, /* sq_ass_item */
  NULL,                                 /* *was* sq_ass_slice */
  (objobjproc)pyprim_prop_collection_contains, /* sq_contains */
  (binaryfunc)NULL,                            /* sq_inplace_concat */
  (ssizeargfunc)NULL,                          /* sq_inplace_repeat */
};

static PySequenceMethods pyprim_prim_as_sequence = {
  NULL, /* Can't set the len otherwise it can evaluate as false */
  NULL, /* sq_concat */
  NULL, /* sq_repeat */
  NULL,
  /* sq_item */                     /* Only set this so PySequence_Check() returns True */
  NULL,                             /* *was* sq_slice */
  NULL,                             /* sq_ass_item */
  NULL,                             /* *was* sq_ass_slice */
  (objobjproc)pyprim_prim_contains, /* sq_contains */
  (binaryfunc)NULL,                 /* sq_inplace_concat */
  (ssizeargfunc)NULL,               /* sq_inplace_repeat */
};

/* NOTE(@campbellbarton): Regarding comparison `__cmp__`:
 * checking the 'ptr->data' matches works in almost all cases,
 * however there are a few RNA properties that are fake sub-structs and
 * share the pointer with the parent, in those cases this happens 'a.b == a'
 * see: r43352 for example.
 *
 * So compare the 'ptr->type' as well to avoid this problem.
 * It's highly unlikely this would happen that 'ptr->data' and 'ptr->prop' would match,
 * but _not_ 'ptr->type' but include this check for completeness. */

static int pyprim_prim_compare(KPy_StagePRIM *a, KPy_StagePRIM *b)
{
  return (((a->ptr.data == b->ptr.data) && (a->ptr.type == b->ptr.type)) ? 0 : -1);
}

static int pyprim_prop_compare(KPy_StagePROP *a, KPy_StagePROP *b)
{
  return (((a->prop == b->prop) && (a->ptr.data == b->ptr.data) && (a->ptr.type == b->ptr.type)) ?
            0 :
            -1);
}

static PyObject *pyprim_prim_richcmp(PyObject *a, PyObject *b, int op)
{
  PyObject *res;
  int ok = -1; /* Zero is true. */

  if (KPy_StagePRIM_Check(a) && KPy_StagePRIM_Check(b)) {
    ok = pyprim_prim_compare((KPy_StagePRIM *)a, (KPy_StagePRIM *)b);
  }

  switch (op) {
    case Py_NE:
      ok = !ok;
      ATTR_FALLTHROUGH;
    case Py_EQ:
      res = ok ? Py_False : Py_True;
      break;

    case Py_LT:
    case Py_LE:
    case Py_GT:
    case Py_GE:
      res = Py_NotImplemented;
      break;
    default:
      PyErr_BadArgument();
      return NULL;
  }

  return Py_INCREF_RET(res);
}

static PyObject *pyprim_prop_richcmp(PyObject *a, PyObject *b, int op)
{
  PyObject *res;
  int ok = -1; /* Zero is true. */

  if (KPy_StagePROP_Check(a) && KPy_StagePROP_Check(b)) {
    ok = pyprim_prop_compare((KPy_StagePROP *)a, (KPy_StagePROP *)b);
  }

  switch (op) {
    case Py_NE:
      ok = !ok;
      ATTR_FALLTHROUGH;
    case Py_EQ:
      res = ok ? Py_False : Py_True;
      break;

    case Py_LT:
    case Py_LE:
    case Py_GT:
    case Py_GE:
      res = Py_NotImplemented;
      break;
    default:
      PyErr_BadArgument();
      return NULL;
  }

  return Py_INCREF_RET(res);
}

/* From Python's meth_hash v3.1.2. */
static long pyprim_prop_hash(KPy_StagePROP *self)
{
  long x, y;
  if (self->ptr.data == NULL) {
    x = 0;
  } else {
    x = _Py_HashPointer(self->ptr.data);
    if (x == -1) {
      return -1;
    }
  }
  y = _Py_HashPointer((void *)(self->prop));
  if (y == -1) {
    return -1;
  }
  x ^= y;
  if (x == -1) {
    x = -2;
  }
  return x;
}

static int deferred_register_prop(KrakenPRIM *sprim, PyObject *key, PyObject *item);
static PyObject *pyprim_prop_array_subscript_slice(KPy_StagePropARRAY *self,
                                                   KrakenPRIM *ptr,
                                                   KrakenPROP *prop,
                                                   Py_ssize_t start,
                                                   Py_ssize_t stop,
                                                   Py_ssize_t length);

static int kpy_class_validate(KrakenPRIM *ptr, void *py_data, int *have_function)
{
  // return kpy_class_validate_recursive(dummyptr, dummyptr->type, py_data, have_function);
  return 1;
}

static int kpy_class_call(kContext *C, KrakenPRIM *ptr, KrakenFUNC *func, ParameterList *parms)
{
  return 1;
}

static void kpy_class_free(void *pyob_ptr) {}

struct KPy_TypesModule_State
{
  /** `LUXO_KrakenSTAGE`. */
  KrakenPRIM ptr;
  /** `LUXO_KrakenSTAGE.objects`, exposed as `kpy.types` */
  KrakenPROP prop;
};

PyDoc_STRVAR(pyprim_prop_path_from_id_doc,
             ".. method:: path_from_id()\n"
             "\n"
             "   Returns the data path from the ID to this property (string).\n"
             "\n"
             "   :return: The path from :class:`kpy.types.kpy_struct.id_data` to this property.\n"
             "   :rtype: str\n");
static PyObject *pyprim_prop_path_from_id(KPy_StagePROP *self)
{
  const char *path;
  KrakenPROP *prop = self->prop;
  PyObject *ret;

  path = LUXO_path_from_ID_to_property(&self->ptr, self->prop);

  if (path == NULL) {
    PyErr_Format(PyExc_ValueError,
                 "%.200s.%.200s.path_from_id() does not support path creation for this type",
                 LUXO_prim_identifier(self->ptr.type).data(),
                 LUXO_prop_identifier(prop).data());
    return NULL;
  }

  ret = PyUnicode_FromString(path);
  MEM_freeN((void *)path);

  return ret;
}

PyDoc_STRVAR(pyprim_prop_as_bytes_doc,
             ".. method:: as_bytes()\n"
             "\n"
             "   Returns this string property as a byte rather than a Python string.\n"
             "\n"
             "   :return: The string as bytes.\n"
             "   :rtype: bytes\n");
static PyObject *pyprim_prop_as_bytes(KPy_StagePROP *self)
{

  if (LUXO_prop_type(self->prop) != PROP_STRING) {
    PyErr_Format(PyExc_TypeError,
                 "%.200s.%.200s.as_bytes() must be a string",
                 LUXO_prim_identifier(self->ptr.type).data(),
                 LUXO_prop_identifier(self->prop).data());
    return NULL;
  }

  PyObject *ret;
  KrakenPROP *prop;
  char buf_fixed[256], *buf;
  int buf_len;

  prop = self->prop;
  buf_len = prop->GetPath().GetString().length();
  buf = KLI_strdup(self->ptr.GetAttribute(prop->GetName()).GetPath().GetText());

  ret = PyBytes_FromStringAndSize(buf, buf_len);

  if (buf_fixed != buf) {
    MEM_freeN(buf);
  }

  return ret;
}

PyDoc_STRVAR(pyprim_prop_update_doc,
             ".. method:: update()\n"
             "\n"
             "   Execute the properties update callback.\n"
             "\n"
             "   .. note::\n"
             "      This is called when assigning a property,\n"
             "      however in rare cases it's useful to call explicitly.\n");
static PyObject *pyprim_prop_update(KPy_StagePROP *self)
{
  LUXO_prop_update(KPY_context_get(), &self->ptr, self->prop);
  Py_RETURN_NONE;
}

/* A bit of a kludge, make a list out of a collection or array,
 * then return the list's iter function, not especially fast, but convenient for now. */
static PyObject *pyprim_prop_array_iter(KPy_StagePropARRAY *self)
{
  /* Try get values from a collection. */
  PyObject *ret;
  PyObject *iter = NULL;
  int len;

  PYSTAGE_PROP_CHECK_OBJ((KPy_StagePROP *)self);

  len = pyprim_prop_array_length(self);
  ret = pyprim_prop_array_subscript_slice(self, &self->ptr, self->prop, 0, len, len);

  /* we know this is a list so no need to PyIter_Check
   * otherwise it could be NULL (unlikely) if conversion failed */
  if (ret) {
    iter = PyObject_GetIter(ret);
    Py_DECREF(ret);
  }

  return iter;
}

static PyObject *pyprim_prop_collection_iter(KPy_StagePROP *self);

PyDoc_STRVAR(pyprim_prop_collection_values_doc,
             ".. method:: values()\n"
             "\n"
             "   Return the values of collection\n"
             "   (matching Python's dict.values() functionality).\n"
             "\n"
             "   :return: the members of this collection.\n"
             "   :rtype: list\n");
static PyObject *pyprim_prop_collection_values(KPy_StagePROP *self)
{
  /* Re-use slice. */
  return pyprim_prop_collection_subscript_slice(self, 0, PY_SSIZE_T_MAX);
}

#ifndef USE_PYRNA_ITER
static PyObject *pyprim_prop_collection_iter(KPy_StagePROP *self)
{
  /* Try get values from a collection. */
  PyObject *ret;
  PyObject *iter = NULL;
  ret = pyprim_prop_collection_values(self);

  /* we know this is a list so no need to PyIter_Check
   * otherwise it could be NULL (unlikely) if conversion failed */
  if (ret) {
    iter = PyObject_GetIter(ret);
    Py_DECREF(ret);
  }

  return iter;
}
#endif /* # !USE_PYRNA_ITER */

static bool foreach_attr_type(KPy_StagePROP *self,
                              const char *attr,
                              /* Values to assign. */
                              RawPropertyType *r_raw_type,
                              int *r_attr_tot,
                              bool *r_attr_signed)
{
  KrakenPROP *prop;
  bool attr_ok = true;
  *r_raw_type = PROP_RAW_UNSET;
  *r_attr_tot = 0;
  *r_attr_signed = false;

  /* NOTE: this is fail with zero length lists, so don't let this get called in that case. */
  LUXO_PROP_BEGIN(&self->ptr, itemptr, self->prop)
  {
    prop = LUXO_prim_find_property(&itemptr, attr);
    if (prop) {
      *r_raw_type = LUXO_prop_raw_type(prop);
      *r_attr_tot = LUXO_prop_array_length(&itemptr, prop);
      *r_attr_signed = (LUXO_prop_subtype(prop) != PROP_UNSIGNED);
    } else {
      attr_ok = false;
    }
    break;
  }
  LUXO_PROP_END;

  return attr_ok;
}

/* pyprim_prop_collection_foreach_get/set both use this. */
static int foreach_parse_args(KPy_StagePROP *self,
                              PyObject *args,
                              /* Values to assign. */
                              const char **r_attr,
                              PyObject **r_seq,
                              int *r_tot,
                              int *r_size,
                              RawPropertyType *r_raw_type,
                              int *r_attr_tot,
                              bool *r_attr_signed)
{
#if 0
  int array_tot;
  int target_tot;
#endif

  *r_size = *r_attr_tot = 0;
  *r_attr_signed = false;
  *r_raw_type = PROP_RAW_UNSET;

  if (!PyArg_ParseTuple(args, "sO:foreach_get/set", r_attr, r_seq)) {
    return -1;
  }

  if (!PySequence_Check(*r_seq) && PyObject_CheckBuffer(*r_seq)) {
    PyErr_Format(
      PyExc_TypeError,
      "foreach_get/set expected second argument to be a sequence or buffer, not a %.200s",
      Py_TYPE(*r_seq)->tp_name);
    return -1;
  }

  /* TODO: buffer may not be a sequence! array.array() is though. */
  *r_tot = PySequence_Size(*r_seq);

  if (*r_tot > 0) {
    if (!foreach_attr_type(self, *r_attr, r_raw_type, r_attr_tot, r_attr_signed)) {
      PyErr_Format(PyExc_AttributeError,
                   "foreach_get/set '%.200s.%200s[...]' elements have no attribute '%.200s'",
                   LUXO_prim_identifier(self->ptr.type).data(),
                   LUXO_prop_identifier(self->prop).data(),
                   *r_attr);
      return -1;
    }
    *r_size = LUXO_raw_type_sizeof(*r_raw_type);

#if 0 /* Works fine, but not strictly needed. \
       * we could allow LUXO_prop_collection_raw_* to do the checks */
    if ((*r_attr_tot) < 1) {
      *r_attr_tot = 1;
    }

    if (LUXO_prop_type(self->prop) == PROP_COLLECTION) {
      array_tot = LUXO_prop_collection_length(&self->ptr, self->prop);
    }
    else {
      array_tot = LUXO_prop_array_length(&self->ptr, self->prop);
    }

    target_tot = array_tot * (*r_attr_tot);

    /* rna_access.c - rna_raw_access(...) uses this same method. */
    if (target_tot != (*r_tot)) {
      PyErr_Format(PyExc_TypeError,
                   "foreach_get(attr, sequence) sequence length mismatch given %d, needed %d",
                   *r_tot,
                   target_tot);
      return -1;
    }
#endif
  }

  /* Check 'r_attr_tot' otherwise we don't know if any values were set.
   * This isn't ideal because it means running on an empty list may
   * fail silently when it's not compatible. */
  if (*r_size == 0 && *r_attr_tot != 0) {
    PyErr_SetString(PyExc_AttributeError, "attribute does not support foreach method");
    return -1;
  }
  return 0;
}

static bool foreach_compat_buffer(RawPropertyType raw_type, int attr_signed, const char *format)
{
  const char f = format ? *format : 'B'; /* B is assumed when not set */

  switch (raw_type) {
    case PROP_RAW_CHAR:
      if (attr_signed) {
        return (f == 'b') ? 1 : 0;
      } else {
        return (f == 'B') ? 1 : 0;
      }
    case PROP_RAW_SHORT:
      if (attr_signed) {
        return (f == 'h') ? 1 : 0;
      } else {
        return (f == 'H') ? 1 : 0;
      }
    case PROP_RAW_INT:
      if (attr_signed) {
        return (f == 'i') ? 1 : 0;
      } else {
        return (f == 'I') ? 1 : 0;
      }
    case PROP_RAW_BOOLEAN:
      return (f == '?') ? 1 : 0;
    case PROP_RAW_FLOAT:
      return (f == 'f') ? 1 : 0;
    case PROP_RAW_DOUBLE:
      return (f == 'd') ? 1 : 0;
    case PROP_RAW_UNSET:
      return 0;
  }

  return 0;
}

static PyObject *foreach_getset(KPy_StagePROP *self, PyObject *args, int set)
{
  PyObject *item = NULL;
  int i = 0, ok = 0;
  bool buffer_is_compat;
  void *array = NULL;

  /* Get/set both take the same args currently. */
  const char *attr;
  PyObject *seq;
  int tot, size, attr_tot;
  bool attr_signed;
  RawPropertyType raw_type;

  if (foreach_parse_args(self,
                         args,
                         &attr,
                         &seq,
                         &tot,
                         &size,
                         &raw_type,
                         &attr_tot,
                         &attr_signed) == -1) {
    return NULL;
  }

  if (tot == 0) {
    Py_RETURN_NONE;
  }

  if (set) { /* Get the array from python. */
    buffer_is_compat = false;
    if (PyObject_CheckBuffer(seq)) {
      Py_buffer buf;
      PyObject_GetBuffer(seq, &buf, PyBUF_SIMPLE | PyBUF_FORMAT);

      /* Check if the buffer matches. */

      buffer_is_compat = foreach_compat_buffer(raw_type, attr_signed, buf.format);

      if (buffer_is_compat) {
        #if 0
        ok = LUXO_prop_collection_raw_set(NULL, &self->ptr, self->prop, attr, buf.buf, raw_type, tot);
        #endif /* 0 */
      }

      PyBuffer_Release(&buf);
    }

    /* Could not use the buffer, fallback to sequence. */
    if (!buffer_is_compat) {
      array = PyMem_Malloc(size * tot);

      for (; i < tot; i++) {
        item = PySequence_GetItem(seq, i);
        switch (raw_type) {
          case PROP_RAW_CHAR:
            ((char *)array)[i] = (char)PyLong_AsLong(item);
            break;
          case PROP_RAW_SHORT:
            ((short *)array)[i] = (short)PyLong_AsLong(item);
            break;
          case PROP_RAW_INT:
            ((int *)array)[i] = (int)PyLong_AsLong(item);
            break;
          case PROP_RAW_BOOLEAN:
            ((bool *)array)[i] = (int)PyLong_AsLong(item) != 0;
            break;
          case PROP_RAW_FLOAT:
            ((float *)array)[i] = (float)PyFloat_AsDouble(item);
            break;
          case PROP_RAW_DOUBLE:
            ((double *)array)[i] = (double)PyFloat_AsDouble(item);
            break;
          case PROP_RAW_UNSET:
            /* Should never happen. */
            KLI_assert_msg(0, "Invalid array type - set");
            break;
        }

        Py_DECREF(item);
      }

      #if 0
      ok = LUXO_prop_collection_raw_set(NULL, &self->ptr, self->prop, attr, array, raw_type, tot);
      #endif /* 0 */
    }
  } else {
    buffer_is_compat = false;
    if (PyObject_CheckBuffer(seq)) {
      Py_buffer buf;
      PyObject_GetBuffer(seq, &buf, PyBUF_SIMPLE | PyBUF_FORMAT);

      /* Check if the buffer matches, TODO: signed/unsigned types. */

      buffer_is_compat = foreach_compat_buffer(raw_type, attr_signed, buf.format);

      if (buffer_is_compat) {
        #if 0
        ok = LUXO_prop_collection_raw_get(NULL, &self->ptr, self->prop, attr, buf.buf, raw_type, tot);
        #endif /* 0 */
      }

      PyBuffer_Release(&buf);
    }

    /* Could not use the buffer, fallback to sequence. */
    if (!buffer_is_compat) {
      array = PyMem_Malloc(size * tot);

      #if 0
      ok = LUXO_prop_collection_raw_get(NULL, &self->ptr, self->prop, attr, array, raw_type, tot);
      #endif /* 0 */

      if (!ok) {
        /* Skip the loop. */
        i = tot;
      }

      for (; i < tot; i++) {

        switch (raw_type) {
          case PROP_RAW_CHAR:
            item = PyLong_FromLong((long)((char *)array)[i]);
            break;
          case PROP_RAW_SHORT:
            item = PyLong_FromLong((long)((short *)array)[i]);
            break;
          case PROP_RAW_INT:
            item = PyLong_FromLong((long)((int *)array)[i]);
            break;
          case PROP_RAW_FLOAT:
            item = PyFloat_FromDouble((double)((float *)array)[i]);
            break;
          case PROP_RAW_DOUBLE:
            item = PyFloat_FromDouble((double)((double *)array)[i]);
            break;
          case PROP_RAW_BOOLEAN:
            item = PyBool_FromLong((long)((bool *)array)[i]);
            break;
          default: /* PROP_RAW_UNSET */
            /* Should never happen. */
            KLI_assert_msg(0, "Invalid array type - get");
            item = Py_None;
            Py_INCREF(item);
            break;
        }

        PySequence_SetItem(seq, i, item);
        Py_DECREF(item);
      }
    }
  }

  if (array) {
    PyMem_Free(array);
  }

  if (PyErr_Occurred()) {
    /* Maybe we could make our own error. */
    PyErr_Print();
    PyErr_SetString(PyExc_TypeError, "couldn't access the py sequence");
    return NULL;
  }
  if (!ok) {
    PyErr_SetString(PyExc_RuntimeError, "internal error setting the array");
    return NULL;
  }

  Py_RETURN_NONE;
}

PyDoc_STRVAR(pyprim_prop_collection_foreach_get_doc,
             ".. method:: foreach_get(attr, seq)\n"
             "\n"
             "   This is a function to give fast access to attributes within a collection.\n");
static PyObject *pyprim_prop_collection_foreach_get(KPy_StagePROP *self, PyObject *args)
{
  PYSTAGE_PROP_CHECK_OBJ(self);

  return foreach_getset(self, args, 0);
}

PyDoc_STRVAR(pyprim_prop_collection_foreach_set_doc,
             ".. method:: foreach_set(attr, seq)\n"
             "\n"
             "   This is a function to give fast access to attributes within a collection.\n");
static PyObject *pyprim_prop_collection_foreach_set(KPy_StagePROP *self, PyObject *args)
{
  PYSTAGE_PROP_CHECK_OBJ(self);

  return foreach_getset(self, args, 1);
}

static PyObject *pyprop_array_foreach_getset(KPy_StagePropARRAY *self,
                                             PyObject *args,
                                             const bool do_set)
{
  PyObject *item = NULL;
  Py_ssize_t i, seq_size, size;
  void *array = NULL;
  const PropertyType prop_type = LUXO_prop_type(self->prop);

  /* Get/set both take the same args currently. */
  PyObject *seq;

  if (!ELEM(prop_type, PROP_INT, PROP_FLOAT)) {
    PyErr_Format(PyExc_TypeError, "foreach_get/set available only for int and float");
    return NULL;
  }

  if (!PyArg_ParseTuple(args, "O:foreach_get/set", &seq)) {
    return NULL;
  }

  if (!PySequence_Check(seq) && PyObject_CheckBuffer(seq)) {
    PyErr_Format(
      PyExc_TypeError,
      "foreach_get/set expected second argument to be a sequence or buffer, not a %.200s",
      Py_TYPE(seq)->tp_name);
    return NULL;
  }

  size = pyprim_prop_array_length(self);
  seq_size = PySequence_Size(seq);

  if (size != seq_size) {
    PyErr_Format(PyExc_TypeError, "expected sequence size %d, got %d", size, seq_size);
    return NULL;
  }

  Py_buffer buf;
  if (PyObject_GetBuffer(seq, &buf, PyBUF_SIMPLE | PyBUF_FORMAT) == -1) {
    PyErr_Clear();

    switch (prop_type) {
      case PROP_INT:
        array = PyMem_Malloc(sizeof(int) * size);
        if (do_set) {
          for (i = 0; i < size; i++) {
            item = PySequence_GetItem(seq, i);
            ((int *)array)[i] = (int)PyLong_AsLong(item);
            Py_DECREF(item);
          }

          LUXO_prop_int_set_array(&self->ptr, self->prop, static_cast<int *>(array));
        } else {
          LUXO_prop_int_get_array(&self->ptr, self->prop, static_cast<int *>(array));

          for (i = 0; i < size; i++) {
            item = PyLong_FromLong((long)((int *)array)[i]);
            PySequence_SetItem(seq, i, item);
            Py_DECREF(item);
          }
        }

        break;
      case PROP_FLOAT:
        array = PyMem_Malloc(sizeof(float) * size);
        if (do_set) {
          for (i = 0; i < size; i++) {
            item = PySequence_GetItem(seq, i);
            ((float *)array)[i] = (float)PyFloat_AsDouble(item);
            Py_DECREF(item);
          }

          LUXO_prop_float_set_array(&self->ptr, self->prop, static_cast<float *>(array));
        } else {
          LUXO_prop_float_get_array(&self->ptr, self->prop, static_cast<float *>(array));

          for (i = 0; i < size; i++) {
            item = PyFloat_FromDouble((double)((float *)array)[i]);
            PySequence_SetItem(seq, i, item);
            Py_DECREF(item);
          }
        }
        break;
      case PROP_BOOLEAN:
      case PROP_STRING:
      case PROP_ENUM:
      case PROP_POINTER:
      case PROP_COLLECTION:
        /* Should never happen. */
        KLI_assert_unreachable();
        break;
    }

    PyMem_Free(array);

    if (PyErr_Occurred()) {
      /* Maybe we could make our own error. */
      PyErr_Print();
      PyErr_SetString(PyExc_TypeError, "couldn't access the py sequence");
      return NULL;
    }
  } else {
    const char f = buf.format ? buf.format[0] : 0;
    if ((prop_type == PROP_INT && (buf.itemsize != sizeof(int) || !ELEM(f, 'l', 'i'))) ||
        (prop_type == PROP_FLOAT && (buf.itemsize != sizeof(float) || f != 'f'))) {
      PyBuffer_Release(&buf);
      PyErr_Format(PyExc_TypeError, "incorrect sequence item type: %s", buf.format);
      return NULL;
    }

    switch (prop_type) {
      case PROP_INT:
        if (do_set) {
          LUXO_prop_int_set_array(&self->ptr, self->prop, static_cast<int *>(buf.buf));
        } else {
          LUXO_prop_int_get_array(&self->ptr, self->prop, static_cast<int *>(buf.buf));
        }
        break;
      case PROP_FLOAT:
        if (do_set) {
          LUXO_prop_float_set_array(&self->ptr, self->prop, static_cast<float *>(buf.buf));
        } else {
          LUXO_prop_float_get_array(&self->ptr, self->prop, static_cast<float *>(buf.buf));
        }
        break;
      case PROP_BOOLEAN:
      case PROP_STRING:
      case PROP_ENUM:
      case PROP_POINTER:
      case PROP_COLLECTION:
        /* Should never happen. */
        KLI_assert_unreachable();
        break;
    }

    PyBuffer_Release(&buf);
  }

  Py_RETURN_NONE;
}

PyDoc_STRVAR(pyprim_prop_array_foreach_get_doc,
             ".. method:: foreach_get(seq)\n"
             "\n"
             "   This is a function to give fast access to array data.\n");
static PyObject *pyprim_prop_array_foreach_get(KPy_StagePropARRAY *self, PyObject *args)
{
  PYSTAGE_PROP_CHECK_OBJ((KPy_StagePROP *)self);

  return pyprop_array_foreach_getset(self, args, false);
}

PyDoc_STRVAR(pyprim_prop_array_foreach_set_doc,
             ".. method:: foreach_set(seq)\n"
             "\n"
             "   This is a function to give fast access to array data.\n");
static PyObject *pyprim_prop_array_foreach_set(KPy_StagePropARRAY *self, PyObject *args)
{
  PYSTAGE_PROP_CHECK_OBJ((KPy_StagePROP *)self);

  return pyprop_array_foreach_getset(self, args, true);
}

static void pyprim_dir_members_prim(PyObject *list, KrakenPRIM *ptr)
{
  const char *idname;

  /* For looping over attributes and functions. */
  KrakenPRIM tptr;
  KrakenPROP *iterprop;

  {
    LUXO_pointer_create(NULL, &PRIM_KrakenPRIM, ptr->type, &tptr);
    iterprop = LUXO_prim_find_property(&tptr, "functions");

    LUXO_PROP_BEGIN(&tptr, itemptr, iterprop)
    {
      KrakenFUNC *func = static_cast<KrakenFUNC *>(itemptr.data);
      if (LUXO_function_defined(func)) {
        idname = LUXO_function_identifier(static_cast<KrakenFUNC *>(itemptr.data));
        PyList_APPEND(list, PyUnicode_FromString(idname));
      }
    }
    LUXO_PROP_END;
  }

  {
    /*
     * Collect RNA attributes
     */
    char name[256], *nameptr;
    int namelen;

    iterprop = LUXO_prim_iterator_property(ptr->type);

    LUXO_PROP_BEGIN(ptr, itemptr, iterprop)
    {
      /* Custom-properties are exposed using `__getitem__`, exclude from `__dir__`. */
      KrakenPRIM *magic = static_cast<KrakenPRIM *>(itemptr.data);
      if (magic && (magic->GetPrim() != UsdPrim())) {
        continue;
      }
      namelen = itemptr.GetName().GetString().length();
      nameptr = KLI_strdup(itemptr.GetName().data());
      KLI_strncpy(name, nameptr, sizeof(name));


      if (nameptr) {
        PyList_APPEND(list, PyUnicode_FromStringAndSize(nameptr, namelen));

        if (name != nameptr) {
          MEM_freeN(nameptr);
        }
      }
    }
    LUXO_PROP_END;
  }
}

static void pyprim_dir_members_py__add_keys(PyObject *list, PyObject *dict)
{
  PyObject *list_tmp;

  list_tmp = PyDict_Keys(dict);
  PyList_SetSlice(list, INT_MAX, INT_MAX, list_tmp);
  Py_DECREF(list_tmp);
}

static void pyprim_dir_members_py(PyObject *list, PyObject *self)
{
  PyObject *dict;
  PyObject **dict_ptr;

  dict_ptr = _PyObject_GetDictPtr((PyObject *)self);

  if (dict_ptr && (dict = *dict_ptr)) {
    pyprim_dir_members_py__add_keys(list, dict);
  }

  dict = ((PyTypeObject *)Py_TYPE(self))->tp_dict;
  if (dict) {
    pyprim_dir_members_py__add_keys(list, dict);
  }

  /* Since this is least common case, handle it last. */
  if (KPy_StagePROP_Check(self)) {
    KPy_StagePROP *self_prop = (KPy_StagePROP *)self;
    if (LUXO_prop_type(self_prop->prop) == PROP_COLLECTION) {
      KrakenPRIM r_ptr;

      if (LUXO_prop_collection_type_get(&self_prop->ptr, self_prop->prop, &r_ptr)) {
        PyObject *cls = pyprim_prim_Subtype(&r_ptr); /* borrows */
        dict = ((PyTypeObject *)cls)->tp_dict;
        pyprim_dir_members_py__add_keys(list, dict);
        Py_DECREF(cls);
      }
    }
  }
}

static PyObject *pyprim_prop_dir(KPy_StagePROP *self)
{
  PyObject *ret;
  KrakenPRIM r_ptr;

  /* Include this in case this instance is a subtype of a Python class
   * In these instances we may want to return a function or variable provided by the subtype. */
  ret = PyList_New(0);

  if (!KPy_StagePROP_CheckExact(self)) {
    pyprim_dir_members_py(ret, (PyObject *)self);
  }

  if (LUXO_prop_type(self->prop) == PROP_COLLECTION) {
    if (LUXO_prop_collection_type_get(&self->ptr, self->prop, &r_ptr)) {
      pyprim_dir_members_prim(ret, &r_ptr);
    }
  }

  return ret;
}

static PyObject *pyprim_func_to_py(const KrakenPRIM *ptr, KrakenFUNC *func)
{
  KPy_KrakenFUNC *pyfunc = (KPy_KrakenFUNC *)PyObject_NEW(KPy_KrakenFUNC, &pyprim_func_Type);
  pyfunc->ptr = *ptr;
  pyfunc->func = func;
  return (PyObject *)pyfunc;
}

static PyObject *pyprim_prop_collection_getattro(KPy_StagePROP *self, PyObject *pyname)
{
  const char *name = PyUnicode_AsUTF8(pyname);

  if (name == NULL) {
    PyErr_SetString(PyExc_AttributeError, "kpy_prop_collection: __getattr__ must be a string");
    return NULL;
  }
  if (name[0] != '_') {
    PyObject *ret;
    KrakenPROP *prop;
    KrakenFUNC *func;

    KrakenPRIM r_ptr;
    if (LUXO_prop_collection_type_get(&self->ptr, self->prop, &r_ptr)) {
      if ((prop = LUXO_prim_find_property(&r_ptr, name))) {
        ret = pyprim_prop_to_py(&r_ptr, prop);

        return ret;
      }
      if ((func = LUXO_prim_find_function(r_ptr.type, name))) {
        PyObject *self_collection = pyprim_prim_CreatePyObject(&r_ptr);
        ret = pyprim_func_to_py(&((KPy_DummyStagePRIM *)self_collection)->ptr, func);
        Py_DECREF(self_collection);

        return ret;
      }
    }
  }

#if 0
  return PyObject_GenericGetAttr((PyObject *)self, pyname);
#else
  {
    /* Could just do this except for 1 awkward case.
     * `PyObject_GenericGetAttr((PyObject *)self, pyname);`
     * so as to support `kpy.data.library.load()` */

    PyObject *ret = PyObject_GenericGetAttr((PyObject *)self, pyname);

    if (ret == NULL && name[0] != '_') { /* Avoid inheriting `__call__` and similar. */
      /* Since this is least common case, handle it last. */
      KrakenPRIM r_ptr;
      if (LUXO_prop_collection_type_get(&self->ptr, self->prop, &r_ptr)) {
        PyObject *cls;

        PyObject *error_type, *error_value, *error_traceback;
        PyErr_Fetch(&error_type, &error_value, &error_traceback);

        cls = pyprim_prim_Subtype(&r_ptr);
        ret = PyObject_GenericGetAttr(cls, pyname);
        Py_DECREF(cls);

        /* Restore the original error. */
        if (ret == NULL) {
          PyErr_Restore(error_type, error_value, error_traceback);
        } else {
          if (Py_TYPE(ret) == &PyMethodDescr_Type) {
            PyMethodDef *m = ((PyMethodDescrObject *)ret)->d_method;
            /* TODO: #METH_CLASS */
            if (m->ml_flags & METH_STATIC) {
              /* Keep 'ret' as-is. */
            } else {
              Py_DECREF(ret);
              ret = PyCMethod_New(m, (PyObject *)self, NULL, NULL);
            }
          }
        }
      }
    }

    return ret;
  }
#endif
}

PyDoc_STRVAR(pyprim_prop_collection_keys_doc,
             ".. method:: keys()\n"
             "\n"
             "   Return the identifiers of collection members\n"
             "   (matching Python's dict.keys() functionality).\n"
             "\n"
             "   :return: the identifiers for each member of this collection.\n"
             "   :rtype: list of strings\n");
static PyObject *pyprim_prop_collection_keys(KPy_StagePROP *self)
{
  PyObject *ret = PyList_New(0);
  char name[256], *nameptr;
  int namelen;

  LUXO_PROP_BEGIN (&self->ptr, itemptr, self->prop) {
    nameptr = LUXO_prim_name_get_alloc(&itemptr, name, sizeof(name), &namelen);

    if (nameptr) {
      PyList_APPEND(ret, PyUnicode_FromStringAndSize(nameptr, namelen));

      if (name != nameptr) {
        MEM_freeN(nameptr);
      }
    }
  }
  LUXO_PROP_END;

  return ret;
}

static struct PyMethodDef pyprim_prop_methods[] = {
  {"path_from_id",
   (PyCFunction)pyprim_prop_path_from_id,
   METH_NOARGS,                                                    pyprim_prop_path_from_id_doc},
  {"as_bytes",     (PyCFunction)pyprim_prop_as_bytes, METH_NOARGS, pyprim_prop_as_bytes_doc    },
  {"update",       (PyCFunction)pyprim_prop_update,   METH_NOARGS, pyprim_prop_update_doc      },
  {"__dir__",      (PyCFunction)pyprim_prop_dir,      METH_NOARGS, NULL                        },
  {NULL,           NULL,                              0,           NULL                        },
};

static struct PyMethodDef pyprim_prop_array_methods[] = {
  {"foreach_get",
   (PyCFunction)pyprim_prop_array_foreach_get,
   METH_VARARGS,                                  pyprim_prop_array_foreach_get_doc},
  {"foreach_set",
   (PyCFunction)pyprim_prop_array_foreach_set,
   METH_VARARGS,                                  pyprim_prop_array_foreach_set_doc},

  {NULL,          NULL,                        0, NULL                             },
};

static struct PyMethodDef pyprim_prop_collection_methods[] = {
  {"foreach_get",
   (PyCFunction)pyprim_prop_collection_foreach_get,
   METH_VARARGS,                                                          pyprim_prop_collection_foreach_get_doc},
  {"foreach_set",
   (PyCFunction)pyprim_prop_collection_foreach_set,
   METH_VARARGS,                                                          pyprim_prop_collection_foreach_set_doc},

  {"keys",        (PyCFunction)pyprim_prop_collection_keys, METH_NOARGS,  pyprim_prop_collection_keys_doc       },
  // {"items",
  //  (PyCFunction)pyprim_prop_collection_items,
  //  METH_NOARGS,                                                           pyprim_prop_collection_items_doc      },
  {"values",
   (PyCFunction)pyprim_prop_collection_values,
   METH_NOARGS,                                                           pyprim_prop_collection_values_doc     },

  //{"get",         (PyCFunction)pyprim_prop_collection_get,  METH_VARARGS, pyprim_prop_collection_get_doc        },
  //{"find",        (PyCFunction)pyprim_prop_collection_find, METH_O,       pyprim_prop_collection_find_doc       },
  {NULL,          NULL,                                     0,            NULL                                  },
};

// static struct PyMethodDef pyprim_prop_collection_idprop_methods[] = {
//   {"add", (PyCFunction)pyprim_prop_collection_idprop_add, METH_NOARGS, NULL},
//   {"remove", (PyCFunction)pyprim_prop_collection_idprop_remove, METH_O, NULL},
//   {"clear", (PyCFunction)pyprim_prop_collection_idprop_clear, METH_NOARGS, NULL},
//   {"move", (PyCFunction)pyprim_prop_collection_idprop_move, METH_VARARGS, NULL},
//   {NULL, NULL, 0, NULL},
// };

PyDoc_STRVAR(pyprim_prim_get_id_data_doc,
             "The :class:`kpy.types.ID` object this datablock is from or None, (not available for "
             "all data types)");
static PyObject *pyprim_prim_get_id_data(KPy_DummyStagePRIM *self)
{
  /* Used for struct and pointer since both have a ptr. */
  if (self->ptr.owner_id) {
    KrakenPRIM id_ptr;
    LUXO_id_pointer_create((ID *)self->ptr.owner_id, &id_ptr);
    return pyprim_prim_CreatePyObject(&id_ptr);
  }

  Py_RETURN_NONE;
}

PyDoc_STRVAR(pyprim_prim_get_data_doc,
             "The data this property is using, *type* :class:`kpy.types.kpy_struct`");
static PyObject *pyprim_prim_get_data(KPy_DummyStagePRIM *self)
{
  return pyprim_prim_CreatePyObject(&self->ptr);
}

PyDoc_STRVAR(pyprim_prim_get_prim_type_doc, "The property type for introspection");
static PyObject *pyprim_prim_get_prim_type(KPy_StagePROP *self)
{
  KrakenPRIM tptr;
  LUXO_pointer_create(NULL, &PRIM_KrakenPROP, self->prop, &tptr);
  return pyprim_prim_Subtype(&tptr);
}

/*****************************************************************************/
/* Python attributes get/set structure:                                      */
/*****************************************************************************/

static PyGetSetDef pyprim_prop_getseters[] = {
  {"id_data",   (getter)pyprim_prim_get_id_data, (setter)NULL, pyprim_prim_get_id_data_doc, NULL},
  {"data",      (getter)pyprim_prim_get_data,    (setter)NULL, pyprim_prim_get_data_doc,    NULL},
  {"prim_type",
   (getter)pyprim_prim_get_prim_type,
   (setter)NULL,
   pyprim_prim_get_prim_type_doc,                                                           NULL},
  {NULL,        NULL,                            NULL,         NULL,                        NULL}  /* Sentinel */
};

static void pyprim_prop_array_dealloc(KPy_StagePROP *self)
{
#ifdef USE_WEAKREFS
  if (self->in_weakreflist != NULL) {
    PyObject_ClearWeakRefs((PyObject *)self);
  }
#endif
  /* NOTE: for subclassed PyObjects calling PyObject_DEL() directly crashes. */
  Py_TYPE(self)->tp_free(self);
}

/**
 * only needed for sub-typing, so a new class gets a valid #KPy_StagePRIM
 * TODO: also accept useful args.
 */
static PyObject *pyprim_prim_new(PyTypeObject *type, PyObject *args, PyObject *UNUSED(kwds))
{
  if (PyTuple_GET_SIZE(args) == 1) {
    KPy_StagePRIM *base = (KPy_StagePRIM *)PyTuple_GET_ITEM(args, 0);
    if (Py_TYPE(base) == type) {
      Py_INCREF(base);
      return (PyObject *)base;
    }
    if (PyType_IsSubtype(Py_TYPE(base), &pyprim_prim_Type)) {
      /* this almost never runs, only when using user defined subclasses of built-in object.
       * this isn't common since it's NOT related to registerable subclasses. eg:
       *
       *  >>> class MyObSubclass(kpy.types.Object):
       *  ...     def test_func(self):
       *  ...         print(100)
       *  ...
       *  >>> myob = MyObSubclass(kpy.context.object)
       *  >>> myob.test_func()
       *  100
       *
       * Keep this since it could be useful.
       */
      KPy_StagePRIM *ret;
      if ((ret = (KPy_StagePRIM *)type->tp_alloc(type, 0))) {
        ret->ptr = base->ptr;
#ifdef USE_PYLUXO_STRUCT_REFERENCE
        /* #PyType_GenericAlloc will have set tracking.
         * We only want tracking when `StructRNA.reference` has been set. */
        PyObject_GC_UnTrack(ret);
#endif
      }
      /* Pass on exception & NULL if tp_alloc fails. */
      return (PyObject *)ret;
    }

    /* Error, invalid type given. */
    PyErr_Format(PyExc_TypeError,
                 "kpy_struct.__new__(type): type '%.200s' is not a subtype of kpy_struct",
                 type->tp_name);
    return NULL;
  }

  PyErr_Format(PyExc_TypeError, "kpy_struct.__new__(type): expected a single argument");
  return NULL;
}

/**
 * Only needed for sub-typing, so a new class gets a valid #KPy_StagePRIM
 * TODO: also accept useful args.
 */
static PyObject *pyprim_prop_new(PyTypeObject *type, PyObject *args, PyObject *UNUSED(kwds))
{
  KPy_StagePROP *base;

  if (!PyArg_ParseTuple(args, "O!:kpy_prop.__new__", &pyprim_prop_Type, &base)) {
    return NULL;
  }

  if (type == Py_TYPE(base)) {
    return Py_INCREF_RET((PyObject *)base);
  }
  if (PyType_IsSubtype(type, &pyprim_prop_Type)) {
    KPy_StagePROP *ret = (KPy_StagePROP *)type->tp_alloc(type, 0);
    ret->ptr = base->ptr;
    ret->prop = base->prop;
    return (PyObject *)ret;
  }

  PyErr_Format(PyExc_TypeError,
               "kpy_prop.__new__(type): type '%.200s' is not a subtype of kpy_prop",
               type->tp_name);
  return NULL;
}


static PyObject *pyprim_prop_array_getattro(KPy_StagePROP *self, PyObject *pyname)
{
  return PyObject_GenericGetAttr((PyObject *)self, pyname);
}


/* --------------- setattr------------------------------------------- */
static int pyprim_prop_collection_setattro(KPy_StagePROP *self, PyObject *pyname, PyObject *value)
{
  const char *name = PyUnicode_AsUTF8(pyname);
  KrakenPROP *prop;
  KrakenPRIM r_ptr;

#ifdef USE_PEDANTIC_WRITE
  if (prim_disallow_writes && prim_id_write_error(&self->ptr, pyname)) {
    return -1;
  }
#endif /* USE_PEDANTIC_WRITE */

  if (name == NULL) {
    PyErr_SetString(PyExc_AttributeError, "kpy_prop: __setattr__ must be a string");
    return -1;
  }
  if (value == NULL) {
    PyErr_SetString(PyExc_AttributeError, "kpy_prop: del not supported");
    return -1;
  }
  if (LUXO_prop_collection_type_get(&self->ptr, self->prop, &r_ptr)) {
    if ((prop = LUXO_prim_find_property(&r_ptr, name))) {
      /* pyprim_py_to_prop sets its own exceptions. */
      PyErr_Format(PyExc_AttributeError, "kpy_prop_collection: attribute \"%.200s\" (setattr) not yet implemented", name);
      return -1;
      //return pyprim_py_to_prop(&r_ptr, prop, NULL, value, "KPy_StagePROP - Attribute (setattr):");
    }
  }

  PyErr_Format(PyExc_AttributeError, "kpy_prop_collection: attribute \"%.200s\" not found", name);
  return -1;
}


PyTypeObject pyprim_prim_meta_idprop_Type = {
  PyVarObject_HEAD_INIT(NULL, 0) "kpy_struct_meta_idprop", /* tp_name */

  /* NOTE! would be PyTypeObject, but subtypes of Type must be PyHeapTypeObject's */
  sizeof(PyHeapTypeObject), /* tp_basicsize */

  0, /* tp_itemsize */
  /* methods */
  NULL, /* tp_dealloc */
  0,    /* tp_vectorcall_offset */
  NULL, /* getattrfunc tp_getattr; */
  NULL, /* setattrfunc tp_setattr; */
  NULL,
  /* tp_compare */ /* deprecated in Python 3.0! */
  NULL,            /* tp_repr */

  /* Method suites for standard classes */
  NULL, /* PyNumberMethods *tp_as_number; */
  NULL, /* PySequenceMethods *tp_as_sequence; */
  NULL, /* PyMappingMethods *tp_as_mapping; */

  /* More standard operations (here for binary compatibility) */
  NULL,                                                       /* hashfunc tp_hash; */
  NULL,                                                       /* ternaryfunc tp_call; */
  NULL,                                                       /* reprfunc tp_str; */
  NULL /* (getattrofunc) pyprim_prim_meta_idprop_getattro */, /* getattrofunc tp_getattro; */

  NULL,  // (setattrofunc)pyprim_prim_meta_idprop_setattro,             /* setattrofunc
         // tp_setattro; */


  /* Functions to access object as input/output buffer */
  NULL, /* PyBufferProcs *tp_as_buffer; */

  /*** Flags to define presence of optional/expanded features ***/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* long tp_flags; */

  NULL, /*  char *tp_doc;  Documentation string */
  /*** Assigned meaning in release 2.0 ***/
  /* call function for all accessible objects */
  NULL, /* traverseproc tp_traverse; */

  /* delete references to contained objects */
  NULL, /* inquiry tp_clear; */

  /***  Assigned meaning in release 2.1 ***/
  /*** rich comparisons ***/
  NULL, /* richcmpfunc tp_richcompare; */

  /***  weak reference enabler ***/
  0, /* long tp_weaklistoffset; */

  /*** Added in release 2.2 ***/
  /*   Iterators */
  NULL, /* getiterfunc tp_iter; */
  NULL, /* iternextfunc tp_iternext; */

  /*** Attribute descriptor and subclassing stuff ***/
  NULL, /* struct PyMethodDef *tp_methods; */
  NULL, /* struct PyMemberDef *tp_members; */
  NULL, /* struct PyGetSetDef *tp_getset; */
#if defined(_MSC_VER)
  NULL, /* defer assignment */
#else
  &PyType_Type, /* struct _typeobject *tp_base; */
#endif
  NULL, /* PyObject *tp_dict; */
  NULL, /* descrgetfunc tp_descr_get; */
  NULL, /* descrsetfunc tp_descr_set; */
  0,    /* long tp_dictoffset; */
  NULL, /* initproc tp_init; */
  NULL, /* allocfunc tp_alloc; */
  NULL, /* newfunc tp_new; */
  /*  Low-level free-memory routine */
  NULL, /* freefunc tp_free; */
  /* For PyObject_IS_GC */
  NULL, /* inquiry tp_is_gc; */
  NULL, /* PyObject *tp_bases; */
  /* method resolution order */
  NULL, /* PyObject *tp_mro; */
  NULL, /* PyObject *tp_cache; */
  NULL, /* PyObject *tp_subclasses; */
  NULL, /* PyObject *tp_weaklist; */
  NULL,
};

PyTypeObject pyprim_prim_Type = {
  PyVarObject_HEAD_INIT(NULL, 0) "kpy_struct", /* tp_name */
  sizeof(KPy_StagePRIM),                       /* tp_basicsize */
  0,                                           /* tp_itemsize */
  /* methods */
  NULL,  //(destructor)pyprim_prim_dealloc, /* tp_dealloc */
  0,     /* tp_vectorcall_offset */
  NULL,  /* getattrfunc tp_getattr; */
  NULL,  /* setattrfunc tp_setattr; */
  NULL,
  /* tp_compare */ /* DEPRECATED in Python 3.0! */
  NULL,            //(reprfunc)pyprim_prim_repr, /* tp_repr */

  /* Method suites for standard classes */

  NULL,  /* PyNumberMethods *tp_as_number; */
  NULL,  //&pyprim_prim_as_sequence, /* PySequenceMethods *tp_as_sequence; */
  NULL,  //&pyprim_prim_as_mapping,  /* PyMappingMethods *tp_as_mapping; */

  /* More standard operations (here for binary compatibility) */

  NULL,  //(hashfunc)pyprim_prim_hash,         /* hashfunc tp_hash; */
  NULL,  /* ternaryfunc tp_call; */
  NULL,  //(reprfunc)pyprim_prim_str,          /* reprfunc tp_str; */
  NULL,  //(getattrofunc)pyprim_prim_getattro, /* getattrofunc tp_getattro; */
  NULL,  //(setattrofunc)pyprim_prim_setattro, /* setattrofunc tp_setattro; */

  /* Functions to access object as input/output buffer */
  NULL, /* PyBufferProcs *tp_as_buffer; */

  /*** Flags to define presence of optional/expanded features ***/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE
#ifdef USE_PYLUXO_STRUCT_REFERENCE
    | Py_TPFLAGS_HAVE_GC
#endif
  , /* long tp_flags; */

  NULL, /*  char *tp_doc;  Documentation string */
/*** Assigned meaning in release 2.0 ***/
/* call function for all accessible objects */
#ifdef USE_PYLUXO_STRUCT_REFERENCE
  NULL,  //(traverseproc)pyprim_prim_traverse, /* traverseproc tp_traverse; */

  /* delete references to contained objects */
  NULL,  //(inquiry)pyprim_prim_clear, /* inquiry tp_clear; */
#else
  NULL,         /* traverseproc tp_traverse; */

  /* delete references to contained objects */
  NULL, /* inquiry tp_clear; */
#endif /* !USE_PYLUXO_STRUCT_REFERENCE */

  /***  Assigned meaning in release 2.1 ***/
  /*** rich comparisons ***/
  NULL,  //(richcmpfunc)pyprim_prim_richcmp, /* richcmpfunc tp_richcompare; */

/***  weak reference enabler ***/
#ifdef USE_WEAKREFS
  offsetof(KPy_StagePRIM, in_weakreflist), /* long tp_weaklistoffset; */
#else
  0,
#endif
  /*** Added in release 2.2 ***/
  /*   Iterators */
  NULL, /* getiterfunc tp_iter; */
  NULL, /* iternextfunc tp_iternext; */

  /*** Attribute descriptor and subclassing stuff ***/
  NULL,  // pyprim_prim_methods,   /* struct PyMethodDef *tp_methods; */
  NULL,  /* struct PyMemberDef *tp_members; */
  NULL,  // pyprim_prim_getseters, /* struct PyGetSetDef *tp_getset; */
  NULL,  /* struct _typeobject *tp_base; */
  NULL,  /* PyObject *tp_dict; */
  NULL,  /* descrgetfunc tp_descr_get; */
  NULL,  /* descrsetfunc tp_descr_set; */
  0,     /* long tp_dictoffset; */
  NULL,  /* initproc tp_init; */
  NULL,  /* allocfunc tp_alloc; */
  NULL,  // pyprim_prim_new,       /* newfunc tp_new; */
  /*  Low-level free-memory routine */
  NULL, /* freefunc tp_free; */
  /* For PyObject_IS_GC */
  NULL, /* inquiry tp_is_gc; */
  NULL, /* PyObject *tp_bases; */
  /* method resolution order */
  NULL, /* PyObject *tp_mro; */
  NULL, /* PyObject *tp_cache; */
  NULL, /* PyObject *tp_subclasses; */
  NULL, /* PyObject *tp_weaklist; */
  NULL,
};

/*-----------------------KPy_KrakenPROP method def------------------------------*/
PyTypeObject pyprim_prop_Type = {
  PyVarObject_HEAD_INIT(NULL, 0) "kpy_prop", /* tp_name */
  sizeof(KPy_StagePROP),                     /* tp_basicsize */
  0,                                         /* tp_itemsize */
  /* methods */
  (destructor)pyprim_prop_dealloc, /* tp_dealloc */
  0,                               /* tp_vectorcall_offset */
  NULL,                            /* getattrfunc tp_getattr; */
  NULL,                            /* setattrfunc tp_setattr; */
  NULL,
  /* tp_compare */            /* DEPRECATED in Python 3.0! */
  (reprfunc)pyprim_prop_repr, /* tp_repr */

  /* Method suites for standard classes */

  NULL, /* PyNumberMethods *tp_as_number; */
  NULL, /* PySequenceMethods *tp_as_sequence; */
  NULL, /* PyMappingMethods *tp_as_mapping; */

  /* More standard operations (here for binary compatibility) */

  (hashfunc)pyprim_prop_hash, /* hashfunc tp_hash; */
  NULL,                       /* ternaryfunc tp_call; */
  (reprfunc)pyprim_prop_str,  /* reprfunc tp_str; */

  /* will only use these if this is a subtype of a py class */
  NULL, /* getattrofunc tp_getattro; */
  NULL, /* setattrofunc tp_setattro; */

  /* Functions to access object as input/output buffer */
  NULL, /* PyBufferProcs *tp_as_buffer; */

  /*** Flags to define presence of optional/expanded features ***/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* long tp_flags; */

  NULL, /*  char *tp_doc;  Documentation string */
  /*** Assigned meaning in release 2.0 ***/
  /* call function for all accessible objects */
  NULL, /* traverseproc tp_traverse; */

  /* delete references to contained objects */
  NULL, /* inquiry tp_clear; */

  /***  Assigned meaning in release 2.1 ***/
  /*** rich comparisons ***/
  (richcmpfunc)pyprim_prop_richcmp, /* richcmpfunc tp_richcompare; */

/***  weak reference enabler ***/
#ifdef USE_WEAKREFS
  offsetof(KPy_KrakenPROP, in_weakreflist), /* long tp_weaklistoffset; */
#else
  0,
#endif

  /*** Added in release 2.2 ***/
  /*   Iterators */
  NULL, /* getiterfunc tp_iter; */
  NULL, /* iternextfunc tp_iternext; */

  /*** Attribute descriptor and subclassing stuff ***/
  pyprim_prop_methods,   /* struct PyMethodDef *tp_methods; */
  NULL,                  /* struct PyMemberDef *tp_members; */
  pyprim_prop_getseters, /* struct PyGetSetDef *tp_getset; */
  NULL,                  /* struct _typeobject *tp_base; */
  NULL,                  /* PyObject *tp_dict; */
  NULL,                  /* descrgetfunc tp_descr_get; */
  NULL,                  /* descrsetfunc tp_descr_set; */
  0,                     /* long tp_dictoffset; */
  NULL,                  /* initproc tp_init; */
  NULL,                  /* allocfunc tp_alloc; */
  pyprim_prop_new,       /* newfunc tp_new; */
  /*  Low-level free-memory routine */
  NULL, /* freefunc tp_free; */
  /* For PyObject_IS_GC */
  NULL, /* inquiry tp_is_gc; */
  NULL, /* PyObject *tp_bases; */
  /* method resolution order */
  NULL, /* PyObject *tp_mro; */
  NULL, /* PyObject *tp_cache; */
  NULL, /* PyObject *tp_subclasses; */
  NULL, /* PyObject *tp_weaklist; */
  NULL,
};

PyTypeObject pyprim_prop_array_Type = {
  PyVarObject_HEAD_INIT(NULL, 0) "kpy_prop_array", /* tp_name */
  sizeof(KPy_StagePropARRAY),                      /* tp_basicsize */
  0,                                               /* tp_itemsize */
  /* methods */
  (destructor)pyprim_prop_array_dealloc, /* tp_dealloc */
  0,                                     /* tp_vectorcall_offset */
  NULL,                                  /* getattrfunc tp_getattr; */
  NULL,                                  /* setattrfunc tp_setattr; */
  NULL,
  /* tp_compare */                  /* DEPRECATED in Python 3.0! */
  (reprfunc)pyprim_prop_array_repr, /* tp_repr */

  /* Method suites for standard classes */

  &pyprim_prop_array_as_number,   /* PyNumberMethods *tp_as_number; */
  &pyprim_prop_array_as_sequence, /* PySequenceMethods *tp_as_sequence; */
  &pyprim_prop_array_as_mapping,  /* PyMappingMethods *tp_as_mapping; */

  /* More standard operations (here for binary compatibility) */

  NULL, /* hashfunc tp_hash; */
  NULL, /* ternaryfunc tp_call; */
  NULL, /* reprfunc tp_str; */

  /* will only use these if this is a subtype of a py class */
  (getattrofunc)pyprim_prop_array_getattro, /* getattrofunc tp_getattro; */
  NULL,                                     /* setattrofunc tp_setattro; */

  /* Functions to access object as input/output buffer */
  NULL, /* PyBufferProcs *tp_as_buffer; */

  /*** Flags to define presence of optional/expanded features ***/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* long tp_flags; */

  NULL, /*  char *tp_doc;  Documentation string */
  /*** Assigned meaning in release 2.0 ***/
  /* call function for all accessible objects */
  NULL, /* traverseproc tp_traverse; */

  /* delete references to contained objects */
  NULL, /* inquiry tp_clear; */

  /***  Assigned meaning in release 2.1 ***/
  /*** rich comparisons (subclassed) ***/
  NULL, /* richcmpfunc tp_richcompare; */

/***  weak reference enabler ***/
#ifdef USE_WEAKREFS
  offsetof(KPy_StagePropARRAY, in_weakreflist), /* long tp_weaklistoffset; */
#else
  0,
#endif
  /*** Added in release 2.2 ***/
  /*   Iterators */
  (getiterfunc)pyprim_prop_array_iter, /* getiterfunc tp_iter; */
  NULL,                                /* iternextfunc tp_iternext; */

  /*** Attribute descriptor and subclassing stuff ***/
  pyprim_prop_array_methods,      /* struct PyMethodDef *tp_methods; */
  NULL,                           /* struct PyMemberDef *tp_members; */
  NULL /*pyprim_prop_getseters*/, /* struct PyGetSetDef *tp_getset; */
  &pyprim_prop_Type,              /* struct _typeobject *tp_base; */
  NULL,                           /* PyObject *tp_dict; */
  NULL,                           /* descrgetfunc tp_descr_get; */
  NULL,                           /* descrsetfunc tp_descr_set; */
  0,                              /* long tp_dictoffset; */
  NULL,                           /* initproc tp_init; */
  NULL,                           /* allocfunc tp_alloc; */
  NULL,                           /* newfunc tp_new; */
  /*  Low-level free-memory routine */
  NULL, /* freefunc tp_free; */
  /* For PyObject_IS_GC */
  NULL, /* inquiry tp_is_gc; */
  NULL, /* PyObject *tp_bases; */
  /* method resolution order */
  NULL, /* PyObject *tp_mro; */
  NULL, /* PyObject *tp_cache; */
  NULL, /* PyObject *tp_subclasses; */
  NULL, /* PyObject *tp_weaklist; */
  NULL,
};

PyTypeObject pyprim_prop_collection_Type = {
  PyVarObject_HEAD_INIT(NULL, 0) "kpy_prop_collection", /* tp_name */
  sizeof(KPy_StagePROP),                                /* tp_basicsize */
  0,                                                    /* tp_itemsize */
  /* methods */
  (destructor)pyprim_prop_dealloc, /* tp_dealloc */
  0,                               /* tp_vectorcall_offset */
  NULL,                            /* getattrfunc tp_getattr; */
  NULL,                            /* setattrfunc tp_setattr; */
  NULL,
  /* tp_compare */ /* DEPRECATED in Python 3.0! */
  NULL,
  /* subclassed */ /* tp_repr */

  /* Method suites for standard classes */

  &pyprim_prop_collection_as_number,   /* PyNumberMethods *tp_as_number; */
  &pyprim_prop_collection_as_sequence, /* PySequenceMethods *tp_as_sequence; */
  &pyprim_prop_collection_as_mapping,  /* PyMappingMethods *tp_as_mapping; */

  /* More standard operations (here for binary compatibility) */

  NULL, /* hashfunc tp_hash; */
  NULL, /* ternaryfunc tp_call; */
  NULL, /* reprfunc tp_str; */

  /* will only use these if this is a subtype of a py class */
  (getattrofunc)pyprim_prop_collection_getattro, /* getattrofunc tp_getattro; */
  (setattrofunc)pyprim_prop_collection_setattro, /* setattrofunc tp_setattro; */

  /* Functions to access object as input/output buffer */
  NULL, /* PyBufferProcs *tp_as_buffer; */

  /*** Flags to define presence of optional/expanded features ***/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* long tp_flags; */

  NULL, /*  char *tp_doc;  Documentation string */
  /*** Assigned meaning in release 2.0 ***/
  /* call function for all accessible objects */
  NULL, /* traverseproc tp_traverse; */

  /* delete references to contained objects */
  NULL, /* inquiry tp_clear; */

  /***  Assigned meaning in release 2.1 ***/
  /*** rich comparisons (subclassed) ***/
  NULL, /* richcmpfunc tp_richcompare; */

/***  weak reference enabler ***/
#ifdef USE_WEAKREFS
  offsetof(KPy_StagePROP, in_weakreflist), /* long tp_weaklistoffset; */
#else
  0,
#endif

  /*** Added in release 2.2 ***/
  /*   Iterators */
  (getiterfunc)pyprim_prop_collection_iter, /* getiterfunc tp_iter; */
  NULL,                                     /* iternextfunc tp_iternext; */

  /*** Attribute descriptor and subclassing stuff ***/
  pyprim_prop_collection_methods, /* struct PyMethodDef *tp_methods; */
  NULL,                           /* struct PyMemberDef *tp_members; */
  NULL /*pyprim_prop_getseters*/, /* struct PyGetSetDef *tp_getset; */
  &pyprim_prop_Type,              /* struct _typeobject *tp_base; */
  NULL,                           /* PyObject *tp_dict; */
  NULL,                           /* descrgetfunc tp_descr_get; */
  NULL,                           /* descrsetfunc tp_descr_set; */
  0,                              /* long tp_dictoffset; */
  NULL,                           /* initproc tp_init; */
  NULL,                           /* allocfunc tp_alloc; */
  NULL,                           /* newfunc tp_new; */
  /*  Low-level free-memory routine */
  NULL, /* freefunc tp_free; */
  /* For PyObject_IS_GC */
  NULL, /* inquiry tp_is_gc; */
  NULL, /* PyObject *tp_bases; */
  /* method resolution order */
  NULL, /* PyObject *tp_mro; */
  NULL, /* PyObject *tp_cache; */
  NULL, /* PyObject *tp_subclasses; */
  NULL, /* PyObject *tp_weaklist; */
  NULL,
};


/* only for add/remove/move methods */
static PyTypeObject pyprim_prop_collection_idprop_Type = {
    PyVarObject_HEAD_INIT(NULL, 0) "kpy_prop_collection_idprop", /* tp_name */
    sizeof(KPy_StagePROP),                                       /* tp_basicsize */
    0,                                                           /* tp_itemsize */
    /* methods */
    (destructor)pyprim_prop_dealloc, /* tp_dealloc */
    0,                               /* tp_vectorcall_offset */
    NULL,                            /* getattrfunc tp_getattr; */
    NULL,                            /* setattrfunc tp_setattr; */
    NULL,
    /* tp_compare */ /* DEPRECATED in Python 3.0! */
    NULL,
    /* subclassed */ /* tp_repr */

    /* Method suites for standard classes */

    NULL, /* PyNumberMethods *tp_as_number; */
    NULL, /* PySequenceMethods *tp_as_sequence; */
    NULL, /* PyMappingMethods *tp_as_mapping; */

    /* More standard operations (here for binary compatibility) */

    NULL, /* hashfunc tp_hash; */
    NULL, /* ternaryfunc tp_call; */
    NULL, /* reprfunc tp_str; */

    /* will only use these if this is a subtype of a py class */
    NULL, /* getattrofunc tp_getattro; */
    NULL, /* setattrofunc tp_setattro; */

    /* Functions to access object as input/output buffer */
    NULL, /* PyBufferProcs *tp_as_buffer; */

    /*** Flags to define presence of optional/expanded features ***/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* long tp_flags; */

    NULL, /*  char *tp_doc;  Documentation string */
    /*** Assigned meaning in release 2.0 ***/
    /* call function for all accessible objects */
    NULL, /* traverseproc tp_traverse; */

    /* delete references to contained objects */
    NULL, /* inquiry tp_clear; */

    /***  Assigned meaning in release 2.1 ***/
    /*** rich comparisons (subclassed) ***/
    NULL, /* richcmpfunc tp_richcompare; */

/***  weak reference enabler ***/
#ifdef USE_WEAKREFS
    offsetof(KPy_StagePROP, in_weakreflist), /* long tp_weaklistoffset; */
#else
    0,
#endif

    /*** Added in release 2.2 ***/
    /*   Iterators */
    NULL, /* getiterfunc tp_iter; */
    NULL, /* iternextfunc tp_iternext; */

    /*** Attribute descriptor and subclassing stuff ***/
    NULL,//pyprim_prop_collection_idprop_methods, /* struct PyMethodDef *tp_methods; */
    NULL,                                  /* struct PyMemberDef *tp_members; */
    NULL /*pyprim_prop_getseters*/,         /* struct PyGetSetDef *tp_getset; */
    &pyprim_prop_collection_Type,          /* struct _typeobject *tp_base; */
    NULL,                                  /* PyObject *tp_dict; */
    NULL,                                  /* descrgetfunc tp_descr_get; */
    NULL,                                  /* descrsetfunc tp_descr_set; */
    0,                                     /* long tp_dictoffset; */
    NULL,                                  /* initproc tp_init; */
    NULL,                                  /* allocfunc tp_alloc; */
    NULL,                                  /* newfunc tp_new; */
    /*  Low-level free-memory routine */
    NULL, /* freefunc tp_free; */
    /* For PyObject_IS_GC */
    NULL, /* inquiry tp_is_gc; */
    NULL, /* PyObject *tp_bases; */
    /* method resolution order */
    NULL, /* PyObject *tp_mro; */
    NULL, /* PyObject *tp_cache; */
    NULL, /* PyObject *tp_subclasses; */
    NULL, /* PyObject *tp_weaklist; */
    NULL,
};

/*-----------------------KPy_StagePROP method def------------------------------*/
PyTypeObject pyprim_func_Type = {
    PyVarObject_HEAD_INIT(NULL, 0) "kpy_func", /* tp_name */
    sizeof(KPy_KrakenFUNC),                    /* tp_basicsize */
    0,                                         /* tp_itemsize */
    /* methods */
    NULL, /* tp_dealloc */
    0,    /* tp_vectorcall_offset */
    NULL, /* getattrfunc tp_getattr; */
    NULL, /* setattrfunc tp_setattr; */
    NULL,
    /* tp_compare */           /* DEPRECATED in Python 3.0! */
    (reprfunc)pyprim_func_repr, /* tp_repr */

    /* Method suites for standard classes */

    NULL, /* PyNumberMethods *tp_as_number; */
    NULL, /* PySequenceMethods *tp_as_sequence; */
    NULL, /* PyMappingMethods *tp_as_mapping; */

    /* More standard operations (here for binary compatibility) */

    NULL,                          /* hashfunc tp_hash; */
    NULL, //(ternaryfunc)pyprim_func_call, /* ternaryfunc tp_call; */
    NULL,                          /* reprfunc tp_str; */

    /* will only use these if this is a subtype of a py class */
    NULL, /* getattrofunc tp_getattro; */
    NULL, /* setattrofunc tp_setattro; */

    /* Functions to access object as input/output buffer */
    NULL, /* PyBufferProcs *tp_as_buffer; */

    /*** Flags to define presence of optional/expanded features ***/
    Py_TPFLAGS_DEFAULT, /* long tp_flags; */

    NULL, /*  char *tp_doc;  Documentation string */
    /*** Assigned meaning in release 2.0 ***/
    /* call function for all accessible objects */
    NULL, /* traverseproc tp_traverse; */

    /* delete references to contained objects */
    NULL, /* inquiry tp_clear; */

    /***  Assigned meaning in release 2.1 ***/
    /*** rich comparisons ***/
    NULL, /* richcmpfunc tp_richcompare; */

/***  weak reference enabler ***/
#ifdef USE_WEAKREFS
    offsetof(KPy_StagePROP, in_weakreflist), /* long tp_weaklistoffset; */
#else
    0,
#endif

    /*** Added in release 2.2 ***/
    /*   Iterators */
    NULL, /* getiterfunc tp_iter; */
    NULL, /* iternextfunc tp_iternext; */

    /*** Attribute descriptor and subclassing stuff ***/
    NULL,                  /* struct PyMethodDef *tp_methods; */
    NULL,                  /* struct PyMemberDef *tp_members; */
    NULL,//pyprim_func_getseters, /* struct PyGetSetDef *tp_getset; */
    NULL,                 /* struct _typeobject *tp_base; */
    NULL,                 /* PyObject *tp_dict; */
    NULL,                 /* descrgetfunc tp_descr_get; */
    NULL,                 /* descrsetfunc tp_descr_set; */
    0,                    /* long tp_dictoffset; */
    NULL,                 /* initproc tp_init; */
    NULL,                 /* allocfunc tp_alloc; */
    NULL,                 /* newfunc tp_new; */
    /*  Low-level free-memory routine */
    NULL, /* freefunc tp_free; */
    /* For PyObject_IS_GC */
    NULL, /* inquiry tp_is_gc; */
    NULL, /* PyObject *tp_bases; */
    /* method resolution order */
    NULL, /* PyObject *tp_mro; */
    NULL, /* PyObject *tp_cache; */
    NULL, /* PyObject *tp_subclasses; */
    NULL, /* PyObject *tp_weaklist; */
    NULL,
};


static PyObject *pyprim_sprim_Subtype(KrakenPRIM *sprim);

/* polymorphism. */
static PyObject *pyprim_sprim_PyBase(KrakenPRIM *sprim)
{
  /* Assume PRIM_struct_py_type_get(sprim) was already checked. */
  KrakenPRIM *base;

  PyObject *py_base = NULL;

  /* Get the base type. */
  base = LUXO_prim_base(sprim);

  if (base && base != sprim) {
    // printf("debug subtype %s %p\n", LUXO_prim_identifier(sprim), sprim);
    py_base = pyprim_sprim_Subtype(base);  //, kpy_types_dict);
    Py_DECREF(py_base);                    /* `sprim` owns, this is only to pass as an argument. */
  }

  if (py_base == NULL) {
    py_base = (PyObject *)&pyprim_prim_Type;
  }

  return py_base;
}

/* Check if we have a native Python subclass, use it when it exists
 * return a borrowed reference. */
static PyObject *kpy_types_dict = NULL;

static PyObject *pyprim_sprim_ExternalType(KrakenPRIM *sprim)
{
  const char *idname = LUXO_prim_identifier(sprim).GetText();
  PyObject *newclass;

  if (kpy_types_dict == NULL) {
    PyObject *kpy_types = PyImport_ImportModuleLevel("kpy_types", NULL, NULL, NULL, 0);

    if (kpy_types == NULL) {
      PyErr_Print();
      PyErr_Clear();
      TF_WARN("failed to find 'kpy_types' module");
      return NULL;
    }
    kpy_types_dict = PyModule_GetDict(kpy_types); /* Borrow. */
    Py_DECREF(kpy_types);                         /* Fairly safe to assume the dict is kept. */
  }

  newclass = PyDict_GetItemString(kpy_types_dict, idname);

  /* Sanity check, could skip this unless in debug mode. */
  if (newclass) {
    PyObject *base_compare = pyprim_sprim_PyBase(sprim);
    /* Can't do this because it gets super-classes values! */
    // PyObject *slots = PyObject_GetAttrString(newclass, "__slots__");
    /* Can do this, but faster not to. */
    // PyObject *bases = PyObject_GetAttrString(newclass, "__bases__");
    PyObject *tp_bases = ((PyTypeObject *)newclass)->tp_bases;
    PyObject *tp_slots = PyDict_GetItem(((PyTypeObject *)newclass)->tp_dict,
                                        kpy_intern_str___slots__);

    if (tp_slots == NULL) {
      TF_WARN("kpy: expected class '%s' to have __slots__ defined, see kpy_types.py", idname);
      newclass = NULL;
    } else if (PyTuple_GET_SIZE(tp_bases)) {
      PyObject *base = PyTuple_GET_ITEM(tp_bases, 0);

      if (base_compare != base) {
        char pyob_info[256];
        PyC_ObSpitStr(pyob_info, sizeof(pyob_info), base_compare);
        TF_WARN("incorrect subclassing of STAGE '%s', expected '%s', see kpy_types.py",
                idname,
                pyob_info);
        newclass = NULL;
      } else {
        TF_MSG_SUCCESS("kpy: STAGE sub-classed: '%s'", idname);
      }
    }
  }

  return newclass;
}

/* polymorphism. */
static PyObject *pyprim_sprim_Subtype(KrakenPRIM *sprim)
{
  PyObject *newclass = NULL;

  /* Stupid/simple case. */
  if (sprim == NULL) {
    newclass = NULL; /* Nothing to do. */
  }                  /* The class may have already been declared & allocated. */
  else if ((newclass = (PyObject *)LUXO_prim_py_type_get(sprim))) {
    Py_INCREF(newclass);
  } /* Check if kpy_types.py module has the class defined in it. */
  else if ((newclass = pyprim_sprim_ExternalType(sprim))) {
    pyprim_subtype_set_prim(newclass, sprim);
    Py_INCREF(newclass);
  } /* create a new class instance with the C api
     * mainly for the purposing of matching the C/RNA type hierarchy */
  else {
    /* subclass equivalents
     * - class myClass(myBase):
     *     some = 'value' # or ...
     * - myClass = type(
     *       name='myClass',
     *       bases=(myBase,), dict={'__module__': 'kpy.types', '__slots__': ()}
     *   )
     */

    /* Assume LUXO_prim_py_type_get(sprim) was already checked. */
    PyObject *py_base = pyprim_sprim_PyBase(sprim);
    PyObject *metaclass;
    const char *idname = LUXO_prim_identifier(sprim).GetText();

    /* Remove `__doc__` for now because we don't need it to generate docs. */
#if 0
    const char *descr = LUXO_prim_ui_description(sprim);
    if (!descr) {
      descr = "(no docs)";
    }
#endif

    if (!PyObject_IsSubclass(py_base, (PyObject *)&pyprim_prim_meta_idprop_Type)) {
      metaclass = (PyObject *)&pyprim_prim_meta_idprop_Type;
    } else {
      metaclass = (PyObject *)&PyType_Type;
    }

    /* Always use O not N when calling, N causes refcount errors. */
#if 0
    newclass = PyObject_CallFunction(
        metaclass, "s(O) {sss()}", idname, py_base, "__module__", "kpy.types", "__slots__");
#else
    {
      /* Longhand of the call above. */
      PyObject *args, *item, *value;
      int ok;

      args = PyTuple_New(3);

      /* arg[0] (name=...) */
      PyTuple_SET_ITEM(args, 0, PyUnicode_FromString(idname));

      /* arg[1] (bases=...) */
      PyTuple_SET_ITEM(args, 1, item = PyTuple_New(1));
      PyTuple_SET_ITEM(item, 0, Py_INCREF_RET(py_base));

      /* arg[2] (dict=...) */
      PyTuple_SET_ITEM(args, 2, item = PyDict_New());
      ok = PyDict_SetItem(item, kpy_intern_str___module__, kpy_intern_str_kpy_types);
      KLI_assert(ok != -1);
      ok = PyDict_SetItem(item, kpy_intern_str___slots__, value = PyTuple_New(0));
      Py_DECREF(value);
      KLI_assert(ok != -1);

      newclass = PyObject_CallObject(metaclass, args);
      Py_DECREF(args);

      (void)ok;
    }
#endif

    /* Newclass will now have 2 ref's, ???,
     * probably 1 is internal since #Py_DECREF here segfaults. */

    // PyC_ObSpit("new class ref", newclass);

    if (newclass) {
      /* sprim owns one, and the other is owned by the caller. */
      pyprim_subtype_set_prim(newclass, sprim);

      /* XXX, adding this back segfaults Blender on load. */
      // Py_DECREF(newclass); /* let sprim own */
    } else {
      /* This should not happen. */
      TF_WARN("failed to register '%s'", idname);
      PyErr_Print();
      PyErr_Clear();
    }
  }

  return newclass;
}

static PyObject *pyprim_prim_Subtype(KrakenPRIM *ptr)
{
  return pyprim_sprim_Subtype(sprim_from_ptr(ptr));
}

int LUXO_prop_collection_lookup_token_index(KrakenPRIM *ptr,
                                            const TfToken &key,
                                            KrakenPRIM *r_ptr,
                                            int *r_index)
{
  UsdPrimRange iter;
  int found = 0;
  int index = 0;

  iter = ptr->GetStage()->Traverse();

  for (const KrakenPRIM &prim : iter) {

    if (prim.GetName() == key) {
      r_ptr = new KrakenPRIM(prim.GetPrim());
      found = 1;
    }

    if (found) {
      break;
    }

    ++index;
  }

  if (iter.empty()) {
    memset(r_ptr, 0, sizeof(*r_ptr));
    *r_index = -1;
  } else {
    *r_index = index;
  }

  return !(iter.empty());
}

int LUXO_prop_collection_lookup_token(KrakenPRIM *ptr, const TfToken &key, KrakenPRIM *r_ptr)
{
  int index;
  return LUXO_prop_collection_lookup_token_index(ptr, key, r_ptr, &index);
}

/**
 * Note that #PROP_NONE is included as a vector subtype. this is because it is handy to
 * have x/y access to fcurve keyframes and other fixed size float arrays of length 2-4.
 */
#define PROP_ALL_VECTOR_SUBTYPES \
  PROP_COORDS:                   \
  case PROP_TRANSLATION:         \
  case PROP_DIRECTION:           \
  case PROP_VELOCITY:            \
  case PROP_ACCELERATION:        \
  case PROP_XYZ:                 \
  case PROP_XYZ_LENGTH

PyObject *pyprim_math_object_from_array(KrakenPRIM *ptr, KrakenPROP *prop)
{
  PyObject *ret = NULL;

#ifdef USE_MATHUTILS
  int subtype, totdim;
  int len;
  const int flag = LUXO_prop_flag(prop);
  const int type = LUXO_prop_type(prop);
  const bool is_thick = (flag & PROP_THICK_WRAP) != 0;

  /* disallow dynamic sized arrays to be wrapped since the size could change
   * to a size mathutils does not support */
  if (flag & PROP_DYNAMIC) {
    return NULL;
  }

  len = LUXO_prop_array_length(ptr, prop);
  if (type == PROP_FLOAT) {
    /* pass */
  } else if (type == PROP_INT) {
    if (is_thick) {
      goto thick_wrap_slice;
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }

  subtype = LUXO_prop_subtype(prop);
  totdim = LUXO_prop_array_dimension(ptr, prop, NULL);

  if (totdim == 1 || (totdim == 2 && subtype == PROP_MATRIX)) {
    if (!is_thick) {
      /* Owned by the mathutils PyObject. */
      ret = pyprim_prop_CreatePyObject(ptr, prop);
    }

    switch (subtype) {
      case PROP_ALL_VECTOR_SUBTYPES:
        if (len >= 2 && len <= 4) {
          if (is_thick) {
            ret = Vector_CreatePyObject(NULL, len, NULL);
            LUXO_prop_float_get_array(ptr, prop, ((VectorObject *)ret)->vec);
          } else {
            PyObject *vec_cb = Vector_CreatePyObject_cb(ret,
                                                        len,
                                                        mathutils_rna_array_cb_index,
                                                        MATHUTILS_CB_SUBTYPE_VEC);
            Py_DECREF(ret); /* The vector owns 'ret' now. */
            ret = vec_cb;   /* Return the vector instead. */
          }
        }
        break;
      case PROP_MATRIX:
        if (len == 16) {
          if (is_thick) {
            ret = Matrix_CreatePyObject(NULL, 4, 4, NULL);
            LUXO_prop_float_get_array(ptr, prop, ((MatrixObject *)ret)->matrix);
          } else {
            PyObject *mat_cb =
              Matrix_CreatePyObject_cb(ret, 4, 4, mathutils_rna_matrix_cb_index, 0);
            Py_DECREF(ret); /* The matrix owns 'ret' now. */
            ret = mat_cb;   /* Return the matrix instead. */
          }
        } else if (len == 9) {
          if (is_thick) {
            ret = Matrix_CreatePyObject(NULL, 3, 3, NULL);
            LUXO_prop_float_get_array(ptr, prop, ((MatrixObject *)ret)->matrix);
          } else {
            PyObject *mat_cb =
              Matrix_CreatePyObject_cb(ret, 3, 3, mathutils_rna_matrix_cb_index, 0);
            Py_DECREF(ret); /* The matrix owns 'ret' now. */
            ret = mat_cb;   /* Return the matrix instead. */
          }
        }
        break;
      case PROP_EULER:
      case PROP_QUATERNION:
        if (len == 3) { /* Euler. */
          if (is_thick) {
            /* Attempt to get order,
             * only needed for thick types since wrapped with update via callbacks. */
            PropertyRNA *prop_eul_order = NULL;
            const short order = pyprim_rotation_euler_order_get(ptr,
                                                                EULER_ORDER_XYZ,
                                                                &prop_eul_order);

            ret = Euler_CreatePyObject(NULL, order, NULL); /* TODO: get order from RNA. */
            LUXO_prop_float_get_array(ptr, prop, ((EulerObject *)ret)->eul);
          } else {
            /* Order will be updated from callback on use. */
            /* TODO: get order from RNA. */
            PyObject *eul_cb = Euler_CreatePyObject_cb(ret,
                                                       EULER_ORDER_XYZ,
                                                       mathutils_rna_array_cb_index,
                                                       MATHUTILS_CB_SUBTYPE_EUL);
            Py_DECREF(ret); /* The euler owns 'ret' now. */
            ret = eul_cb;   /* Return the euler instead. */
          }
        } else if (len == 4) {
          if (is_thick) {
            ret = Quaternion_CreatePyObject(NULL, NULL);
            LUXO_prop_float_get_array(ptr, prop, ((QuaternionObject *)ret)->quat);
          } else {
            PyObject *quat_cb = Quaternion_CreatePyObject_cb(ret,
                                                             mathutils_rna_array_cb_index,
                                                             MATHUTILS_CB_SUBTYPE_QUAT);
            Py_DECREF(ret); /* The quat owns 'ret' now. */
            ret = quat_cb;  /* Return the quat instead. */
          }
        }
        break;
      case PROP_COLOR:
      case PROP_COLOR_GAMMA:
        if (len == 3) { /* Color. */
          if (is_thick) {
            ret = Color_CreatePyObject(NULL, NULL);
            LUXO_prop_float_get_array(ptr, prop, ((ColorObject *)ret)->col);
          } else {
            PyObject *col_cb = Color_CreatePyObject_cb(ret,
                                                       mathutils_rna_array_cb_index,
                                                       MATHUTILS_CB_SUBTYPE_COLOR);
            Py_DECREF(ret); /* The color owns 'ret' now. */
            ret = col_cb;   /* Return the color instead. */
          }
        }
        break;
      default:
        break;
    }
  }

  if (ret == NULL) {
    if (is_thick) {
      /* This is an array we can't reference (since it is not thin wrappable)
       * and cannot be coerced into a mathutils type, so return as a list. */
    thick_wrap_slice:
      ret = pyprim_prop_array_subscript_slice(NULL, ptr, prop, 0, len, len);
    } else {
      ret = pyprim_prop_CreatePyObject(ptr, prop); /* Owned by the mathutils PyObject. */
    }
  }
#else  /* USE_MATHUTILS */
  (void)ptr;
  (void)prop;
#endif /* USE_MATHUTILS */

  return ret;
}

PyObject *pyprim_py_from_array(KrakenPRIM *ptr, KrakenPROP *prop)
{
  PyObject *ret;

  ret = pyprim_math_object_from_array(ptr, prop);

  /* is this a maths object? */
  if (ret) {
    return ret;
  }

  return pyprim_prop_CreatePyObject(ptr, prop);
}

PyObject *pyprim_prop_to_py(KrakenPRIM *ptr, KrakenPROP *prop)
{
  PyObject *ret;
  const int type = LUXO_prop_type(prop);

  if (LUXO_prop_array_check(prop)) {
    return pyprim_py_from_array(ptr, prop);
  }

  /* See if we can coerce into a Python type - 'PropertyType'. */
  switch (type) {
    case PROP_BOOLEAN:
      ret = PyBool_FromLong(LUXO_prop_boolean_get(ptr, prop));
      break;
    case PROP_INT:
      ret = PyLong_FromLong(LUXO_prop_int_get(ptr, prop));
      break;
    case PROP_FLOAT:
      ret = PyFloat_FromDouble(LUXO_prop_float_get(ptr, prop));
      break;
    case PROP_STRING: {
      const int subtype = LUXO_prop_subtype(prop);
      const char *buf;
      int buf_len;
      char buf_fixed[32];

      buf = LUXO_prop_string_get_alloc(ptr, prop, buf_fixed, sizeof(buf_fixed), &buf_len);
#ifdef USE_STRING_COERCE
      /* Only file paths get special treatment, they may contain non UTF-8 chars. */
      if (subtype == PROP_BYTESTRING) {
        ret = PyBytes_FromStringAndSize(buf, buf_len);
      } else if (ELEM(subtype, PROP_FILEPATH, PROP_DIRPATH, PROP_FILENAME)) {
        ret = PyC_UnicodeFromByteAndSize(buf, buf_len);
      } else {
        ret = PyUnicode_FromStringAndSize(buf, buf_len);
      }
#else  /* USE_STRING_COERCE */
      if (subtype == PROP_BYTESTRING) {
        ret = PyBytes_FromStringAndSize(buf, buf_len);
      } else {
        ret = PyUnicode_FromStringAndSize(buf, buf_len);
      }
#endif /* USE_STRING_COERCE */
      if (buf_fixed != buf) {
        MEM_freeN((void *)buf);
      }
      break;
    }
    case PROP_ENUM: {
      PyErr_Format(PyExc_TypeError,
                   "kpy_struct internal error: PROP_ENUM not yet implemented. (pyprim_prop_to_py)",
                   type);
      ret = NULL;
      //ret = pyprim_enum_to_py(ptr, prop, LUXO_prop_enum_get(ptr, prop));
      break;
    }
    case PROP_POINTER: {
      KrakenPRIM newptr;
      newptr = LUXO_prop_pointer_get(ptr, prop);
      if (newptr.data) {
        ret = pyprim_prim_CreatePyObject(&newptr);
      } else {
        ret = Py_None;
        Py_INCREF(ret);
      }
      break;
    }
    case PROP_COLLECTION:
      ret = pyprim_prop_CreatePyObject(ptr, prop);
      break;
    default:
      PyErr_Format(PyExc_TypeError,
                   "kpy_struct internal error: unknown type '%d' (pyprim_prop_to_py)",
                   type);
      ret = NULL;
      break;
  }

  return ret;
}

static PyObject *kpy_types_module_getattro(PyObject *self, PyObject *pyname)
{
  struct KPy_TypesModule_State *state = (KPy_TypesModule_State *)PyModule_GetState(self);
  KrakenPRIM newptr;
  PyObject *ret;
  const char *name = PyUnicode_AsUTF8(pyname);

  if (name == NULL) {
    PyErr_SetString(PyExc_AttributeError, "kpy.types: __getattr__ must be a string");
    ret = NULL;
  } else if (LUXO_prop_collection_lookup_token(&state->ptr, TfToken(name), &newptr)) {
    ret = pyprim_prim_Subtype(&newptr);
    if (ret == NULL) {
      PyErr_Format(PyExc_RuntimeError,
                   "kpy.types.%.200s subtype could not be generated, this is a bug!",
                   PyUnicode_AsUTF8(pyname));
    }
  } else {
#if 0
    PyErr_Format(PyExc_AttributeError,
                 "kpy.types.%.200s PRIM_Struct does not exist",
                 PyUnicode_AsUTF8(pyname));
    return NULL;
#endif
    /* The error raised here will be displayed. */
    ret = PyObject_GenericGetAttr((PyObject *)self, pyname);
  }

  return ret;
}

static PyObject *kpy_types_module_dir(PyObject *self)
{
  KrakenPRIM *root;

  struct KPy_TypesModule_State *state = (KPy_TypesModule_State *)PyModule_GetState(self);
  PyObject *ret = PyList_New(0);

  root = &state->ptr;
  for (const KrakenPRIM &prim : root->GetStage()->Traverse()) {
    PyList_APPEND(ret, PyUnicode_FromString(LUXO_prim_identifier(&prim).GetText()));
  }

  /* Include the modules `__dict__` for Python only types. */
  PyObject *submodule_dict = PyModule_GetDict(self);
  PyObject *key, *value;
  Py_ssize_t pos = 0;
  while (PyDict_Next(submodule_dict, &pos, &key, &value)) {
    PyList_Append(ret, key);
  }
  return ret;
}

static struct PyMethodDef kpy_types_module_methods[] = {
  {"__getattr__", (PyCFunction)kpy_types_module_getattro, METH_O,      NULL},
  {"__dir__",     (PyCFunction)kpy_types_module_dir,      METH_NOARGS, NULL},
  {NULL,          NULL,                                   0,           NULL},
};

PyDoc_STRVAR(kpy_types_module_doc, "Access to internal Kraken types");
static struct PyModuleDef kpy_types_module_def = {
  PyModuleDef_HEAD_INIT,
  "kpy.types",                          /* m_name */
  kpy_types_module_doc,                 /* m_doc */
  sizeof(struct KPy_TypesModule_State), /* m_size */
  kpy_types_module_methods,             /* m_methods */
  NULL,                                 /* m_reload */
  NULL,                                 /* m_traverse */
  NULL,                                 /* m_clear */
  NULL,                                 /* m_free */
};

/**
 * Accessed from Python as 'kpy.types' */
PyObject *KPY_prim_types(void)
{
  PyObject *submodule = PyModule_Create(&kpy_types_module_def);
  KPy_TypesModule_State *state = (KPy_TypesModule_State *)PyModule_GetState(submodule);

  LUXO_kraken_luxo_pointer_create(&state->ptr);
  LUXO_object_find_property(&state->ptr, TfToken("collection:structs:includes"), &state->prop);

  /* Internal base types we have no other accessors for. */
  {
    static PyTypeObject *pyprim_types[] = {
      &pyprim_prim_meta_idprop_Type,
      &pyprim_prim_Type,
      &pyprim_prop_Type,
      // &pyprim_prop_array_Type,
      &pyprim_prop_collection_Type,
      &pyprim_func_Type,
    };

    PyObject *submodule_dict = PyModule_GetDict(submodule);
    for (int i = 0; i < ARRAY_SIZE(pyprim_types); i += 1) {
      PyDict_SetItemString(submodule_dict, pyprim_types[i]->tp_name, (PyObject *)pyprim_types[i]);
    }
  }

  return submodule;
}

void pyprim_subtype_set_prim(PyObject *newclass, KrakenPRIM *sprim)
{
  KrakenPRIM ptr;
  PyObject *item;

  Py_INCREF(newclass);

  if (LUXO_prim_py_type_get(sprim)) {
    PyC_ObSpit("LUXO WAS SET - ", (PyObject *)LUXO_prim_py_type_get(sprim));
  }

  Py_XDECREF(((PyObject *)LUXO_prim_py_type_get(sprim)));

  LUXO_prim_py_type_set(sprim, (void *)newclass); /* Store for later use */

  /* Not 100% needed, but useful,
   * having an instance within a type looks wrong, but this instance _is_ a LUXO type. */

  /* Python deals with the circular reference. */
  LUXO_pointer_create(NULL, &PRIM_Struct, sprim, &ptr);
  item = pyprim_prim_CreatePyObject(&ptr);

  /* NOTE: must set the class not the __dict__ else the internal slots are not updated correctly.
   */
  PyObject_SetAttr(newclass, kpy_intern_str_kr_prim, item);
  Py_DECREF(item);

  /* Add staticmethods and classmethods. */
  {
    const KrakenPRIM func_ptr = KrakenPRIM(sprim);
    const ListBase *lb;
    Link *link;

    lb = LUXO_prim_type_functions(sprim);
    for (link = (Link *)lb->first; link; link = link->next) {
      KrakenFUNC *func = (KrakenFUNC *)link;
      const int flag = LUXO_function_flag(func);
      if ((flag & FUNC_NO_SELF) &&         /* Is staticmethod or classmethod. */
          (flag & FUNC_REGISTER) == false) /* Is not for registration. */
      {
        /* We may want to set the type of this later. */
        PyObject *func_py = pyprim_func_to_py(&func_ptr, func);
        PyObject_SetAttrString(newclass, LUXO_function_identifier(func), func_py);
        Py_DECREF(func_py);
      }
    }
  }

  /* Done with RNA instance. */
}

KrakenPRIM *pyprim_prim_as_sprim(PyObject *self, const bool parent, const char *error_prefix)
{
  KPy_StagePRIM *py_sprim = NULL;
  KrakenPRIM *sprim;

  /* Unfortunately PyObject_GetAttrString won't look up this types tp_dict first :/ */
  if (PyType_Check(self)) {
    py_sprim = (KPy_StagePRIM *)PyDict_GetItem(((PyTypeObject *)self)->tp_dict,
                                               kpy_intern_str_kr_prim);
    Py_XINCREF(py_sprim);
  }

  if (parent) {
    /* be very careful with this since it will return a parent classes sprim.
     * modifying this will do confusing stuff! */
    if (py_sprim == NULL) {
      py_sprim = (KPy_StagePRIM *)PyObject_GetAttr(self, kpy_intern_str_kr_prim);
    }
  }

  if (py_sprim == NULL) {
    PyErr_Format(
      PyExc_RuntimeError,
      "%.200s, missing kr_prim attribute from '%.200s' instance (may not be registered)",
      error_prefix,
      Py_TYPE(self)->tp_name);
    return NULL;
  }

  if (!KPy_StagePRIM_Check(py_sprim)) {
    PyErr_Format(PyExc_TypeError,
                 "%.200s, kr_prim attribute wrong type '%.200s' on '%.200s'' instance",
                 error_prefix,
                 Py_TYPE(py_sprim)->tp_name,
                 Py_TYPE(self)->tp_name);
    Py_DECREF(py_sprim);
    return NULL;
  }

  if (py_sprim->ptr.type != &PRIM_Struct) {
    PyErr_Format(PyExc_TypeError,
                 "%.200s, kr_prim attribute not a PRIM_Struct, on '%.200s'' instance",
                 error_prefix,
                 Py_TYPE(self)->tp_name);
    Py_DECREF(py_sprim);
    return NULL;
  }

  sprim = (KrakenPRIM *)py_sprim->ptr.data;
  Py_DECREF(py_sprim);

  return sprim;
}

/* Orphan functions, not sure where they should go. */
KrakenPRIM *sprim_from_self(PyObject *self, const char *error_prefix)
{

  if (self == NULL) {
    return NULL;
  }
  if (PyCapsule_CheckExact(self)) {
    return (KrakenPRIM *)PyCapsule_GetPointer(self, NULL);
  }
  if (PyType_Check(self) == 0) {
    return NULL;
  }

  /* These cases above not errors, they just mean the type was not compatible
   * After this any errors will be raised in the script */

  PyObject *error_type, *error_value, *error_traceback;
  KrakenPRIM *sprim;

  PyErr_Fetch(&error_type, &error_value, &error_traceback);
  sprim = pyprim_prim_as_sprim(self, false, error_prefix);

  if (!PyErr_Occurred()) {
    PyErr_Restore(error_type, error_value, error_traceback);
  }

  return sprim;
}

static int deferred_register_prop(KrakenPRIM *sprim, PyObject *key, PyObject *item)
{
  if (!KPy_PropDeferred_CheckTypeExact(item)) {
    /* No error, ignoring. */
    return 0;
  }

  /* We only care about results from C which
   * are for sure types, save some time with error */
  PyObject *py_func = ((KPy_PropDeferred *)item)->fn;
  PyObject *py_kw = ((KPy_PropDeferred *)item)->kw;
  PyObject *py_sprim_cobject, *py_ret;

  /* Show the function name in errors to help give context. */
  KLI_assert(PyCFunction_CheckExact(py_func));
  PyMethodDef *py_func_method_def = ((PyCFunctionObject *)py_func)->m_ml;
  const char *func_name = py_func_method_def->ml_name;

  PyObject *args_fake;
  const char *key_str = PyUnicode_AsUTF8(key);

  if (*key_str == '_') {
    PyErr_Format(PyExc_ValueError,
                 "kpy_struct \"%.200s\" registration error: "
                 "'%.200s' %.200s could not register because it starts with an '_'",
                 LUXO_prim_identifier(sprim).data(),
                 key_str,
                 func_name);
    return -1;
  }
  py_sprim_cobject = PyCapsule_New(sprim, NULL, NULL);

  /* Not 100% nice :/, modifies the dict passed, should be ok. */
  PyDict_SetItem(py_kw, kpy_intern_str_attr, key);

  args_fake = PyTuple_New(1);
  PyTuple_SET_ITEM(args_fake, 0, py_sprim_cobject);

  PyObject *type = PyDict_GetItemString(py_kw, "type");
  KrakenPRIM *type_sprim = sprim_from_self(type, "");
  if (type_sprim) {
    // if (!LUXO_prim_idprops_datablock_allowed(sprim) &&
    //     (*(PyCFunctionWithKeywords)PyCFunction_GET_FUNCTION(py_func) == KPy_PointerProperty ||
    //      *(PyCFunctionWithKeywords)PyCFunction_GET_FUNCTION(py_func) == KPy_CollectionProperty)
    //      &&
    //     LUXO_prim_idprops_contains_datablock(type_sprim)) {
    //   PyErr_Format(PyExc_ValueError,
    //                "kpy_struct \"%.200s\" registration error: "
    //                "'%.200s' %.200s could not register because "
    //                "this type doesn't support data-block properties",
    //                LUXO_prim_identifier(sprim),
    //                key_str,
    //                func_name);
    //   return -1;
    // }
  }

  py_ret = PyObject_Call(py_func, args_fake, py_kw);

  if (py_ret) {
    Py_DECREF(py_ret);
    Py_DECREF(args_fake); /* Free's py_sprim_cobject too. */
  } else {
    /* _must_ print before decreffing args_fake. */
    PyErr_Print();
    PyErr_Clear();

    Py_DECREF(args_fake); /* Free's py_sprim_cobject too. */

    PyErr_Format(PyExc_ValueError,
                 "kpy_struct \"%.200s\" registration error: "
                 "'%.200s' %.200s could not register (see previous error)",
                 LUXO_prim_identifier(sprim).data(),
                 key_str,
                 func_name);
    return -1;
  }

  return 0;
}

/**
 * Extract `__annotations__` using `typing.get_type_hints` which handles the delayed evaluation.
 */
static int pyprim_deferred_register_class_from_type_hints(KrakenPRIM *sprim,
                                                          PyTypeObject *py_class)
{
  PyObject *annotations_dict = NULL;

  /* `typing.get_type_hints(py_class)` */
  {
    PyObject *typing_mod = PyImport_ImportModuleLevel("typing", NULL, NULL, NULL, 0);
    if (typing_mod != NULL) {
      PyObject *get_type_hints_fn = PyObject_GetAttrString(typing_mod, "get_type_hints");
      if (get_type_hints_fn != NULL) {
        PyObject *args = PyTuple_New(1);

        PyTuple_SET_ITEM(args, 0, (PyObject *)py_class);
        Py_INCREF(py_class);

        annotations_dict = PyObject_CallObject(get_type_hints_fn, args);

        Py_DECREF(args);
        Py_DECREF(get_type_hints_fn);
      }
      Py_DECREF(typing_mod);
    }
  }

  int ret = 0;
  if (annotations_dict != NULL) {
    if (PyDict_CheckExact(annotations_dict)) {
      PyObject *item, *key;
      Py_ssize_t pos = 0;

      while (PyDict_Next(annotations_dict, &pos, &key, &item)) {
        ret = deferred_register_prop(sprim, key, item);
        if (ret != 0) {
          break;
        }
      }
    } else {
      /* Should never happen, an error won't have been raised, so raise one. */
      PyErr_Format(PyExc_TypeError,
                   "typing.get_type_hints returned: %.200s, expected dict\n",
                   Py_TYPE(annotations_dict)->tp_name);
      ret = -1;
    }

    Py_DECREF(annotations_dict);
  } else {
    KLI_assert(PyErr_Occurred());
    fprintf(stderr, "typing.get_type_hints failed with: %.200s\n", py_class->tp_name);
    ret = -1;
  }

  return ret;
}

static int pyprim_deferred_register_props(KrakenPRIM *sprim, PyObject *class_dict)
{
  PyObject *annotations_dict;
  PyObject *item, *key;
  Py_ssize_t pos = 0;
  int ret = 0;

  /* in both cases PyDict_CheckExact(class_dict) will be true even
   * though Operators have a metaclass dict namespace */
  if ((annotations_dict = PyDict_GetItem(class_dict, kpy_intern_str___annotations__)) &&
      PyDict_CheckExact(annotations_dict)) {
    while (PyDict_Next(annotations_dict, &pos, &key, &item)) {
      ret = deferred_register_prop(sprim, key, item);

      if (ret != 0) {
        break;
      }
    }
  }

  return ret;
}

static int pyprim_deferred_register_class_recursive(KrakenPRIM *sprim, PyTypeObject *py_class)
{
  const int len = PyTuple_GET_SIZE(py_class->tp_bases);
  int i, ret;

  /* First scan base classes for registerable properties. */
  for (i = 0; i < len; i++) {
    PyTypeObject *py_superclass = (PyTypeObject *)PyTuple_GET_ITEM(py_class->tp_bases, i);

    /* the rules for using these base classes are not clear,
     * 'object' is of course not worth looking into and
     * existing subclasses of RNA would cause a lot more dictionary
     * looping then is needed (SomeOperator would scan Operator.__dict__)
     * which is harmless, but not at all useful.
     *
     * So only scan base classes which are not subclasses if pixar types.
     * This best fits having 'mix-in' classes for operators and render engines.
     */
    if (py_superclass != &PyBaseObject_Type &&
        !PyObject_IsSubclass((PyObject *)py_superclass, (PyObject *)&pyprim_prim_Type)) {
      ret = pyprim_deferred_register_class_recursive(sprim, py_superclass);

      if (ret != 0) {
        return ret;
      }
    }
  }

  /* Not register out own properties. */
  /* getattr(..., "__dict__") returns a proxy. */
  return pyprim_deferred_register_props(sprim, py_class->tp_dict);
}

int pyprim_deferred_register_class(KrakenPRIM *sprim, PyTypeObject *py_class)
{
  /**
   * Panels and Menus don't need this
   * save some time and skip the checks here */
  if (!LUXO_prim_idprops_register_check(sprim)) {
    return 0;
  }

#ifdef USE_POSTPONED_ANNOTATIONS
  const bool use_postponed_annotations = true;
#else
  const bool use_postponed_annotations = false;
#endif

  if (use_postponed_annotations) {
    return pyprim_deferred_register_class_from_type_hints(sprim, py_class);
  }
  return pyprim_deferred_register_class_recursive(sprim, py_class);
}


PyDoc_STRVAR(pyprim_register_class_doc,
             ".. method:: register_class(cls)\n"
             "\n"
             "   Register a subclass of a Kraken type class.\n"
             "\n"
             "   :arg cls: Kraken type class in:\n"
             "      :class:`kpy.types.Panel`, :class:`kpy.types.UIList`,\n"
             "      :class:`kpy.types.Menu`, :class:`kpy.types.Header`,\n"
             "      :class:`kpy.types.Operator`, :class:`kpy.types.KeyingSetInfo`,\n"
             "      :class:`kpy.types.RenderEngine`\n"
             "   :type cls: class\n"
             "   :raises ValueError:\n"
             "      if the class is not a subclass of a registerable kraken class.\n"
             "\n"
             "   .. note::\n"
             "\n"
             "      If the class has a *register* class method it will be called\n"
             "      before registration.\n");
PyMethodDef meth_kpy_register_class = {"register_class",
                                       pyprim_register_class,
                                       METH_O,
                                       pyprim_register_class_doc};
static PyObject *pyprim_register_class(PyObject *UNUSED(self), PyObject *py_class)
{
  kContext *C = NULL;
  ReportList reports;
  PrimRegisterFUNC reg;
  KrakenPRIM *sprim;
  KrakenPRIM *sprim_new;
  const char *identifier;
  PyObject *py_cls_meth;
  const char *error_prefix = "register_class(...):";

  if (!PyType_Check(py_class)) {
    PyErr_Format(PyExc_ValueError,
                 "register_class(...): "
                 "expected a class argument, not '%.200s'",
                 Py_TYPE(py_class)->tp_name);
    return NULL;
  }

  if (PyDict_GetItem(((PyTypeObject *)py_class)->tp_dict, kpy_intern_str_kr_prim)) {
    PyErr_Format(PyExc_ValueError,
                 "register_class(...): "
                 "already registered as a subclass '%.200s'",
                 ((PyTypeObject *)py_class)->tp_name);
    return NULL;
  }

  if (!pyprim_write_check()) {
    PyErr_Format(PyExc_RuntimeError,
                 "register_class(...): "
                 "can't run in readonly state '%.200s'",
                 ((PyTypeObject *)py_class)->tp_name);
    return NULL;
  }

  /* WARNING: gets parent classes sprim, only for the register function. */
  sprim = pyprim_prim_as_sprim(py_class, true, "register_class(...):");
  if (sprim == NULL) {
    return NULL;
  }

  /* Fails in some cases, so can't use this check, but would like to :| */
#if 0
  if (LUXO_prim_py_type_get(sprim)) {
    PyErr_Format(PyExc_ValueError,
                 "register_class(...): %.200s's parent class %.200s is already registered, this "
                 "is not allowed",
                 ((PyTypeObject *)py_class)->tp_name,
                 LUXO_prim_identifier(sprim));
    return NULL;
  }
#endif

  /* Check that we have a register callback for this type. */
  reg = LUXO_prim_register(sprim);

  if (!reg) {
    PyErr_Format(PyExc_ValueError,
                 "register_class(...): expected a subclass of a registerable "
                 "LUXO type (%.200s does not support registration)",
                 LUXO_prim_identifier(sprim).GetText());
    return NULL;
  }

  /* Get the context, so register callback can do necessary refreshes. */
  C = KPY_context_get();

  /* Call the register callback with reports & identifier. */
  KKE_reports_init(&reports, RPT_STORE);

  identifier = ((PyTypeObject *)py_class)->tp_name;

  sprim_new = reg(CTX_data_main(C),
                  &reports,
                  py_class,
                  identifier,
                  kpy_class_validate,
                  kpy_class_call,
                  kpy_class_free);

  if (!KLI_listbase_is_empty(&reports.list)) {
    const bool has_error = KPy_reports_to_error(&reports, PyExc_RuntimeError, false);
    if (!has_error) {
      KPy_reports_write_stdout(&reports, error_prefix);
    }
    KKE_reports_clear(&reports);
    if (has_error) {
      return NULL;
    }
  }

  /* Python errors validating are not converted into reports so the check above will fail.
   * the cause for returning NULL will be printed as an error */
  if (sprim_new == NULL) {
    return NULL;
  }

  /* Takes a reference to 'py_class'. */
  pyprim_subtype_set_prim(py_class, sprim_new);

  /* Old sprim still references us, keep the check in case registering somehow can free it. */
  if (LUXO_prim_py_type_get(sprim)) {
    LUXO_prim_py_type_set(sprim, NULL);
#if 0
    /* Should be able to do this XXX since the old RNA adds a new ref. */
    Py_DECREF(py_class);
#endif
  }

  /* Can't use this because it returns a dict proxy
   *
   * item = PyObject_GetAttrString(py_class, "__dict__");
   */
  if (pyprim_deferred_register_class(sprim_new, (PyTypeObject *)py_class) != 0) {
    return NULL;
  }

  /* Call classed register method.
   * Note that zero falls through, no attribute, no error. */
  switch (_PyObject_LookupAttr(py_class, kpy_intern_str_register, &py_cls_meth)) {
    case 1: {
      PyObject *ret = PyObject_CallObject(py_cls_meth, NULL);
      Py_DECREF(py_cls_meth);
      if (ret) {
        Py_DECREF(ret);
      } else {
        return NULL;
      }
      break;
    }
    case -1: {
      return NULL;
    }
  }

  Py_RETURN_NONE;
}

#ifdef USE_PYUSD_OBJECT_REFERENCE
static void pyprim_prim_reference_set(KPy_StagePRIM *self, PyObject *reference)
{
  if (self->reference) {
    PyObject_GC_UnTrack(self);
    Py_CLEAR(self->reference);
  }
  /* Reference is now NULL. */

  if (reference) {
    self->reference = reference;
    Py_INCREF(reference);
    PyObject_GC_Track(self);
  }
}
#endif /* !USE_PYLUXO_STRUCT_REFERENCE */

#ifdef USE_PYUSD_INVALIDATE_WEAKREF
static RHash *id_weakref_pool = nullptr;
static PyObject *id_free_weakref_cb(PyObject *weakinfo_pair, PyObject *weakref);
static PyMethodDef id_free_weakref_cb_def = {"id_free_weakref_cb",
                                             (PyCFunction)id_free_weakref_cb,
                                             METH_O,
                                             NULL};

static RHash *id_weakref_pool_get(const SdfPath &id)
{
  RHash *weakinfo_hash = nullptr;

  if (id_weakref_pool) {
    weakinfo_hash = (RHash *)KLI_rhash_lookup(id_weakref_pool, id.GetAsToken());
  } else {
    /* First time, allocate pool. */
    // id_weakref_pool = KLI_rhash_ptr_new("uni_global_pool");
    // weakinfo_hash = NULL;
  }

  if (weakinfo_hash == NULL) {
    // weakinfo_hash = KLI_rhash_ptr_new("prim_id");
    KLI_rhash_insert(id_weakref_pool, id.GetAsToken(), weakinfo_hash);
  }

  return weakinfo_hash;
}

static void id_weakref_pool_add(const SdfPath &id, KPy_DummyStagePRIM *pyprim)
{
  PyObject *weakref;
  PyObject *weakref_capsule;
  PyObject *weakref_cb_py;

  /* Create a new function instance and insert the list as 'self'
   * so we can remove ourself from it. */
  RHash *weakinfo_hash = id_weakref_pool_get(id); /* New or existing. */

  weakref_capsule = PyCapsule_New(weakinfo_hash, NULL, NULL);
  weakref_cb_py = PyCFunction_New(&id_free_weakref_cb_def, weakref_capsule);
  Py_DECREF(weakref_capsule);

  /* Add weakref to weakinfo_hash list. */
  weakref = PyWeakref_NewRef((PyObject *)pyprim, weakref_cb_py);

  Py_DECREF(weakref_cb_py); /* Function owned by the weakref now. */

  /* Important to add at the end of the hash, since first removal looks at the end. */

  /* Using a hash table as a set, all 'id's are the same. */
  KLI_rhash_insert(weakinfo_hash, id.GetAsToken(), weakref);
  /* weakinfo_hash owns the weakref */
}
#endif /* USE_PYUSD_INVALIDATE_WEAKREF */

void KPY_prim_init(void)
{
/* For some reason MSVC complains of these. */
#if defined(_MSC_VER)
  pyprim_prim_meta_idprop_Type.tp_base = &PyType_Type;
#endif

  /* metaclass */
  if (PyType_Ready(&pyprim_prim_meta_idprop_Type) < 0) {
    return;
  }

  if (PyType_Ready(&pyprim_prim_Type) < 0) {
    return;
  }

  // if (PyType_Ready(&pyprim_prop_Type) < 0) {
  //   return;
  // }

  // if (PyType_Ready(&pyprim_prop_array_Type) < 0) {
  //   return;
  // }

  if (PyType_Ready(&pyprim_prop_collection_Type) < 0) {
    return;
  }

  if (PyType_Ready(&pyprim_prop_collection_idprop_Type) < 0) {
    return;
  }

  if (PyType_Ready(&pyprim_func_Type) < 0) {
    return;
  }

#ifdef USE_PYUSD_ITER
  // if (PyType_Ready(&pyprim_prop_collection_iter_Type) < 0) {
  //   return;
  // }
#endif
}


PyDoc_STRVAR(pyprim_unregister_class_doc,
             ".. method:: unregister_class(cls)\n"
             "\n"
             "   Unload the Python class from kraken.\n"
             "\n"
             "   If the class has an *unregister* class method it will be called\n"
             "   before unregistering.\n");
PyMethodDef meth_kpy_unregister_class = {
  "unregister_class",
  pyprim_unregister_class,
  METH_O,
  pyprim_unregister_class_doc,
};
static PyObject *pyprim_unregister_class(PyObject *UNUSED(self), PyObject *py_class)
{
  kContext *C = NULL;
  PrimUnregisterFUNC unreg;
  UsdPrim sprim;
  PyObject *py_cls_meth;

  Py_RETURN_NONE;
}


/* 'kpy.data' from Python. */
static KrakenPRIM *prim_module_ptr = nullptr;
PyObject *KPY_prim_module(void)
{
  KPy_StagePRIM *pyprim;
  KrakenPRIM ptr;

  /* For now, return the base RNA type rather than a real module. */
  LUXO_main_pointer_create(G_MAIN, &ptr);
  pyprim = (KPy_StagePRIM *)pyprim_prim_CreatePyObject(&ptr);

  prim_module_ptr = &pyprim->ptr;
  return (PyObject *)pyprim;
}

void KPY_update_prim_module(void)
{
  if (prim_module_ptr) {
    prim_module_ptr->data = G_MAIN;
  }
}

void pyprim_alloc_types(void)
{
  // #ifdef DEBUG
  //   PyGILState_STATE gilstate;

  //   KrakenPRIM ptr;
  //   KrakenPROP *prop;

  //   gilstate = PyGILState_Ensure();

  //   /* Avoid doing this lookup for every getattr. */
  //   LUXO_kraken_luxo_pointer_create(&ptr);
  //   prop = LUXO_object_find_property(&ptr, "structs");

  //   USD_PROP_BEGIN (&ptr, itemptr, prop) {
  //     PyObject *item = pyprim_prim_Subtype(&itemptr);
  //     if (item == NULL) {
  //       if (PyErr_Occurred()) {
  //         PyErr_Print();
  //         PyErr_Clear();
  //       }
  //     }
  //     else {
  //       Py_DECREF(item);
  //     }
  //   }
  //   USD_PROP_END;

  //   PyGILState_Release(gilstate);
  // #endif /* DEBUG */
}

/*-----------------------CreatePyObject---------------------------------*/
PyObject *pyprim_prim_CreatePyObject(KrakenPRIM *ptr)
{
  KPy_StagePRIM *pyprim = NULL;

  /* NOTE: don't rely on this to return None since NULL data with a valid type can often crash. */
  if (ptr->data == NULL && ptr->type == NULL) { /* Operator RNA has NULL data. */
    Py_RETURN_NONE;
  }

  void **instance = ptr->data ? LUXO_prim_instance(ptr) : NULL;
  if (instance && *instance) {
    pyprim = (KPy_StagePRIM *)*instance;

    /* Refine may have changed types after the first instance was created. */
    if (ptr->type == pyprim->ptr.type) {
      Py_INCREF(pyprim);
      return (PyObject *)pyprim;
    }

    /* Existing users will need to use 'type_recast' method. */
    Py_DECREF(pyprim);
    *instance = NULL;
    /* Continue as if no instance was made. */
#if 0 /* No need to assign, will be written to next... */
      pyprim = NULL;
#endif
  }

  {
    PyTypeObject *tp = (PyTypeObject *)pyprim_prim_Subtype(ptr);

    if (tp) {
      pyprim = (KPy_StagePRIM *)tp->tp_alloc(tp, 0);
#ifdef USE_PYLUXO_STRUCT_REFERENCE
      /* #PyType_GenericAlloc will have set tracking.
       * We only want tracking when `KrakenSTAGE.reference` has been set. */
      if (pyprim != NULL) {
        PyObject_GC_UnTrack(pyprim);
      }
#endif
      Py_DECREF(tp); /* sprim owns, can't hold a reference. */
    } else {
      TF_WARN("kpy: could not make type '%s'", LUXO_prim_identifier(ptr).GetText());

#ifdef USE_PYLUXO_STRUCT_REFERENCE
      pyprim = (KPyKPy_StagePRIM_StructLUXO *)PyObject_GC_New(KPy_StagePRIM, &pyprim_prim_Type);
#else
      pyprim = (KPy_StagePRIM *)PyObject_New(KPy_StagePRIM, &pyprim_prim_Type);
#endif

#ifdef USE_WEAKREFS
      if (pyprim != NULL) {
        pyprim->in_weakreflist = NULL;
      }
#endif
    }
  }

  if (pyprim == NULL) {
    PyErr_SetString(PyExc_MemoryError, "couldn't create kpy_struct object");
    return NULL;
  }

  /* Kraken's instance owns a reference (to avoid Python freeing it). */
  if (instance) {
    *instance = pyprim;
    Py_INCREF(pyprim);
  }

  pyprim->ptr = *ptr;
#ifdef PYLUXO_FREE_SUPPORT
  pyprim->freeptr = false;
#endif

#ifdef USE_PYLUXO_STRUCT_REFERENCE
  pyprim->reference = NULL;
#endif

  PyC_ObSpit("NewKrakenSTAGE: ", (PyObject *)pyprim);

#ifdef USE_PYLUXO_INVALIDATE_WEAKREF
  if (ptr->owner_id) {
    id_weakref_pool_add(ptr->owner_id, (KPy_DummyStagePRIM *)pyprim);
  }
#endif
  return (PyObject *)pyprim;
}

PyObject *pyprim_prop_CreatePyObject(KrakenPRIM *ptr, KrakenPROP *prop)
{
  KPy_StagePROP *pyprim;

  if (LUXO_prop_array_check(prop) == 0) {
    PyTypeObject *type;

    if (LUXO_prop_type(prop) != PROP_COLLECTION) {
      type = &pyprim_prop_Type;
    } else {
      if ((prop->flag & PROP_IDPROPERTY) == 0) {
        type = &pyprim_prop_collection_Type;
      } else {
        type = &pyprim_prop_collection_idprop_Type;
      }
    }

    pyprim = (KPy_StagePROP *)PyObject_NEW(KPy_StagePROP, type);
#ifdef USE_WEAKREFS
    pyprim->in_weakreflist = NULL;
#endif
  } else {
    pyprim = (KPy_StagePROP *)PyObject_NEW(KPy_StagePropARRAY, &pyprim_prop_array_Type);
    ((KPy_StagePropARRAY *)pyprim)->arraydim = 0;
    ((KPy_StagePropARRAY *)pyprim)->arrayoffset = 0;
#ifdef USE_WEAKREFS
    ((KPy_StagePropARRAY *)pyprim)->in_weakreflist = NULL;
#endif
  }

  if (pyprim == NULL) {
    PyErr_SetString(PyExc_MemoryError, "couldn't create KPy_prim object");
    return NULL;
  }

  pyprim->ptr = *ptr;
  pyprim->prop = prop;

#ifdef USE_PYLUXO_INVALIDATE_WEAKREF
  if (ptr->owner_id) {
    id_weakref_pool_add(ptr->owner_id, (KPy_DummyStagePRIM *)pyprim);
  }
#endif

  return (PyObject *)pyprim;
}

static PyObject *pyprim_kr_owner_id_get(PyObject *UNUSED(self))
{
  const char *name = LUXO_prim_state_owner_get();
  if (name) {
    return PyUnicode_FromString(name);
  }
  Py_RETURN_NONE;
}

static PyObject *pyprim_kr_owner_id_set(PyObject *UNUSED(self), PyObject *value)
{
  const char *name;
  if (value == Py_None) {
    name = NULL;
  } else if (PyUnicode_Check(value)) {
    name = PyUnicode_AsUTF8(value);
  } else {
    PyErr_Format(PyExc_ValueError,
                 "owner_set(...): "
                 "expected None or a string, not '%.200s'",
                 Py_TYPE(value)->tp_name);
    return NULL;
  }
  // LUXO_prim_state_owner_set(name);
  Py_RETURN_NONE;
}

PyMethodDef meth_kpy_owner_id_get = {
  "_kr_owner_id_get",
  (PyCFunction)pyprim_kr_owner_id_get,
  METH_NOARGS,
  NULL,
};
PyMethodDef meth_kpy_owner_id_set = {
  "_kr_owner_id_set",
  (PyCFunction)pyprim_kr_owner_id_set,
  METH_O,
  NULL,
};
