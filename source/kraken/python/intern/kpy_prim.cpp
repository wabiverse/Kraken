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
#define USE_MATHUTILS
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

static bool uni_disallow_writes = false;

#ifdef USE_PEDANTIC_WRITE
bool pyprim_write_check(void)
{
  return !uni_disallow_writes;
}

void pyprim_write_set(bool val)
{
  uni_disallow_writes = !val;
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

static Py_ssize_t pyprim_prop_collection_length(KPy_StagePROP *self);
static Py_ssize_t pyprim_prop_array_length(KPy_StagePropARRAY *self);
static int pyprim_py_to_prop(KrakenPRIM *ptr,
                             KrakenPROP *prop,
                             void *data,
                             PyObject *value,
                             const char *error_prefix);
static int deferred_register_prop(KrakenPRIM *srna, PyObject *key, PyObject *item);

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

PyObject *pyprim_array_index(KrakenPRIM *ptr, KrakenPROP *prop, int index)
{
  PyObject *item;

  switch (LUXO_prop_type(prop)) {
    case PROP_FLOAT:
      item = PyFloat_FromDouble((double)ptr->GetAttribute(prop->GetName())
                                  .GetTypeName()
                                  .GetArrayType()
                                  .GetDimensions()
                                  .d[index]);
      break;
    case PROP_BOOLEAN:
      item = PyBool_FromLong((bool)ptr->GetAttribute(prop->GetName())
                               .GetTypeName()
                               .GetArrayType()
                               .GetDimensions()
                               .d[index]);
      break;
    case PROP_INT:
      item = PyLong_FromLong((int)ptr->GetAttribute(prop->GetName())
                               .GetTypeName()
                               .GetArrayType()
                               .GetDimensions()
                               .d[index]);
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

  PyErr_Format(PyExc_IndexError, "bpy_prop_array[index]: index %d out of range", keynum);
  return NULL;
}

/**
 * TODO: dimensions
 * \note Could also use pyprim_prop_array_to_py_index(self, count) in a loop, but it's much slower
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
  // PYLUXO_PROP_CHECK_OBJ((BPy_KrakenPROP *)self);

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
        LUXO_prop_float_get_array(ptr->GetAttribute(prop->GetName()));

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
          values = PyMem_MALLOC(sizeof(bool) * length);
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
          values = PyMem_MALLOC(sizeof(int) * length);
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
        BLI_assert_msg(0, "Invalid array type");

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
      PyErr_SetString(PyExc_TypeError, "bpy_prop_array[slice]: slice steps not supported");
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

  PyErr_SetString(PyExc_AttributeError, "bpy_prop_array[key]: invalid key, key must be an int");
  return NULL;
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

static struct PyMethodDef pyprim_prop_methods[] = {
  {"path_from_id",
   (PyCFunction)pyprim_prop_path_from_id,
   METH_NOARGS,                                                    pyprim_prop_path_from_id_doc},
  {"as_bytes",     (PyCFunction)pyprim_prop_as_bytes, METH_NOARGS, pyprim_prop_as_bytes_doc    },
  {"update",       (PyCFunction)pyprim_prop_update,   METH_NOARGS, pyprim_prop_update_doc      },
  {"__dir__",      (PyCFunction)pyprim_prop_dir,      METH_NOARGS, NULL                        },
  {NULL,           NULL,                              0,           NULL                        },
};

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
       *  >>> class MyObSubclass(bpy.types.Object):
       *  ...     def test_func(self):
       *  ...         print(100)
       *  ...
       *  >>> myob = MyObSubclass(bpy.context.object)
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
                 "bpy_struct.__new__(type): type '%.200s' is not a subtype of bpy_struct",
                 type->tp_name);
    return NULL;
  }

  PyErr_Format(PyExc_TypeError, "bpy_struct.__new__(type): expected a single argument");
  return NULL;
}

/**
 * Only needed for sub-typing, so a new class gets a valid #KPy_StagePRIM
 * TODO: also accept useful args.
 */
static PyObject *pyprim_prop_new(PyTypeObject *type, PyObject *args, PyObject *UNUSED(kwds))
{
  KPy_StagePROP *base;

  if (!PyArg_ParseTuple(args, "O!:bpy_prop.__new__", &pyprim_prop_Type, &base)) {
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
               "bpy_prop.__new__(type): type '%.200s' is not a subtype of bpy_prop",
               type->tp_name);
  return NULL;
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
  offsetof(BPy_PropertyArrayRNA, in_weakreflist), /* long tp_weaklistoffset; */
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
      // &pyprim_func_Type,
    };

    PyObject *submodule_dict = PyModule_GetDict(submodule);
    for (int i = 0; i < ARRAY_SIZE(pyprim_types); i += 1) {
      PyDict_SetItemString(submodule_dict, pyprim_types[i]->tp_name, (PyObject *)pyprim_types[i]);
    }
  }

  return submodule;
}

// static PyObject *pyprim_func_to_py(const KrakenPRIM *ptr, KrakenPRIM *func)
// {
//   // KPy_KrakenFUNC *pyfunc = (KPy_KrakenFUNC *)PyObject_NEW(KPy_KrakenFUNC,
//   // &pyprim_func_Type); pyfunc->ptr = *ptr; pyfunc->func = (KrakenFUNC *)func; return
//   (PyObject
//   // *)pyfunc;
//   Py_RETURN_NONE;
// }

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
  // {
  // const KrakenPRIM func_ptr(*sprim);

  // auto &lb = LUXO_prim_type_functions(sprim);
  // for (auto link : lb) {
  //   KrakenFUNC *func = (KrakenFUNC *)link;
  //   const int flag = LUXO_function_flag(func);
  //   if ((flag & FUNC_NO_SELF) &&         /* Is staticmethod or classmethod. */
  //       (flag & FUNC_REGISTER) == false) /* Is not for registration. */
  //   {
  //     PyObject *func_py = pyprim_func_to_py(&func_ptr, (KrakenPRIM *)func);
  //     PyObject_SetAttrString(newclass, LUXO_function_identifier(func), func_py);
  //     Py_DECREF(func_py);
  //   }
  // }
  // }

  /* Done with LUXO instance. */
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
                 LUXO_prim_identifier(sprim),
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
                 LUXO_prim_identifier(sprim),
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

  // if (PyType_Ready(&pyprim_prop_collection_idprop_Type) < 0) {
  //   return;
  // }

  // if (PyType_Ready(&pyprim_func_Type) < 0) {
  //   return;
  // }

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
  KPy_StagePROP *pyprop;

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

    pyprim = (KPy_KrakenPROP *)PyObject_NEW(KPy_KrakenPROP, type);
#ifdef USE_WEAKREFS
    pyprim->in_weakreflist = NULL;
#endif
  } else {
    pyprim = (KPy_KrakenPROP *)PyObject_NEW(KPy_StagePropARRAY, &pyprim_prop_array_Type);
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
  // const char *name = LUXO_prim_state_owner_get();
  // if (name) {
  //   return PyUnicode_FromString(name);
  // }
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
