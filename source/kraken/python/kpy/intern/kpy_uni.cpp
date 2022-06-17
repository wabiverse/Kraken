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

#include <Python.h>

#include <float.h> /* FLT_MIN/MAX */
#include <stddef.h>

#include "KLI_utildefines.h"
#include "KLI_string_utils.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_robinhood.h"
#include "KKE_utils.h"

#include "LUXO_access.h"
#include "LUXO_runtime.h"

#include "UNI_object.h"
#include "UNI_wm_types.h"

#include "kpy_interface.h"
#include "kpy_intern_string.h"
#include "kpy_uni.h"

#include <wabi/usd/usd/prim.h>
#include <wabi/base/tf/iterator.h>

#define USE_PEDANTIC_WRITE
#define USE_MATHUTILS
#define USE_STRING_COERCE

#define USE_POSTPONED_ANNOTATIONS

WABI_NAMESPACE_BEGIN

KPy_KrakenPrim *kpy_context_module = nullptr; /* for fast access */

static PyObject *pyuni_register_class(PyObject *self, PyObject *py_class);
static PyObject *pyuni_unregister_class(PyObject *self, PyObject *py_class);

static bool uni_disallow_writes = false;

static bool rna_id_write_error(PointerLUXO *ptr, PyObject *key)
{
  return false;
}

bool pyuni_write_check(void)
{
  return !uni_disallow_writes;
}

void pyuni_write_set(bool val)
{
  uni_disallow_writes = !val;
}

static int kpy_class_validate(KrakenPrim *dummyptr, void *py_data, int *have_function)
{
  // return kpy_class_validate_recursive(dummyptr, dummyptr->type, py_data, have_function);
  return 1;
}

static int kpy_class_call(kContext *C, KrakenPrim *ptr, void *func, UsdAttributeVector parms)
{
  return 1;
}

static void kpy_class_free(void *pyob_ptr) {}

struct KPy_TypesModule_State
{
  /** `LUXO_KrakenLUXO`. */
  PointerLUXO ptr;
  /** `LUXO_KrakenLUXO.objects`, exposed as `kpy.types` */
  PropertyLUXO *prop;
};

static PyObject *kpy_types_module_getattro(PyObject *self, PyObject *pyname)
{
  Py_RETURN_NONE;
}

static PyObject *kpy_types_module_dir(PyObject *self)
{
  Py_RETURN_NONE;
}

PyTypeObject pyuni_struct_meta_idprop_Type = {
  PyVarObject_HEAD_INIT(NULL, 0) "kpy_object_meta_idprop", /* tp_name */

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
  NULL,                                                        /* hashfunc tp_hash; */
  NULL,                                                        /* ternaryfunc tp_call; */
  NULL,                                                        /* reprfunc tp_str; */
  NULL /* (getattrofunc) pyrna_struct_meta_idprop_getattro */, /* getattrofunc tp_getattro; */

  NULL,  // (setattrofunc)pyrna_struct_meta_idprop_setattro,             /* setattrofunc
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

PyTypeObject pyuni_object_Type = {
  PyVarObject_HEAD_INIT(NULL, 0) "kpy_object", /* tp_name */
  sizeof(KPy_KrakenPrim),                      /* tp_basicsize */
  0,                                           /* tp_itemsize */
  /* methods */
  NULL,  //(destructor)pyuni_object_dealloc, /* tp_dealloc */
  0,     /* tp_vectorcall_offset */
  NULL,  /* getattrfunc tp_getattr; */
  NULL,  /* setattrfunc tp_setattr; */
  NULL,
  /* tp_compare */ /* DEPRECATED in Python 3.0! */
  NULL,            //(reprfunc)pyuni_object_repr, /* tp_repr */

  /* Method suites for standard classes */

  NULL,  /* PyNumberMethods *tp_as_number; */
  NULL,  //&pyrna_struct_as_sequence, /* PySequenceMethods *tp_as_sequence; */
  NULL,  //&pyrna_struct_as_mapping,  /* PyMappingMethods *tp_as_mapping; */

  /* More standard operations (here for binary compatibility) */

  NULL,  //(hashfunc)pyuni_object_hash,         /* hashfunc tp_hash; */
  NULL,  /* ternaryfunc tp_call; */
  NULL,  //(reprfunc)pyuni_object_str,          /* reprfunc tp_str; */
  NULL,  //(getattrofunc)pyuni_object_getattro, /* getattrofunc tp_getattro; */
  NULL,  //(setattrofunc)pyuni_object_setattro, /* setattrofunc tp_setattro; */

  /* Functions to access object as input/output buffer */
  NULL, /* PyBufferProcs *tp_as_buffer; */

  /*** Flags to define presence of optional/expanded features ***/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE
#ifdef USE_PYRNA_STRUCT_REFERENCE
    | Py_TPFLAGS_HAVE_GC
#endif
  , /* long tp_flags; */

  NULL, /*  char *tp_doc;  Documentation string */
/*** Assigned meaning in release 2.0 ***/
/* call function for all accessible objects */
#ifdef USE_PYRNA_STRUCT_REFERENCE
  NULL,  //(traverseproc)pyuni_struct_traverse, /* traverseproc tp_traverse; */

  /* delete references to contained objects */
  NULL,  //(inquiry)pyuni_struct_clear, /* inquiry tp_clear; */
#else
  NULL,         /* traverseproc tp_traverse; */

  /* delete references to contained objects */
  NULL, /* inquiry tp_clear; */
#endif /* !USE_PYRNA_STRUCT_REFERENCE */

  /***  Assigned meaning in release 2.1 ***/
  /*** rich comparisons ***/
  NULL,  //(richcmpfunc)pyrna_struct_richcmp, /* richcmpfunc tp_richcompare; */

/***  weak reference enabler ***/
#ifdef USE_WEAKREFS
  offsetof(KPy_KrakenPrim, in_weakreflist), /* long tp_weaklistoffset; */
#else
  0,
#endif
  /*** Added in release 2.2 ***/
  /*   Iterators */
  NULL, /* getiterfunc tp_iter; */
  NULL, /* iternextfunc tp_iternext; */

  /*** Attribute descriptor and subclassing stuff ***/
  NULL,  // pyuni_object_methods,   /* struct PyMethodDef *tp_methods; */
  NULL,  /* struct PyMemberDef *tp_members; */
  NULL,  // pyuni_object_getseters, /* struct PyGetSetDef *tp_getset; */
  NULL,  /* struct _typeobject *tp_base; */
  NULL,  /* PyObject *tp_dict; */
  NULL,  /* descrgetfunc tp_descr_get; */
  NULL,  /* descrsetfunc tp_descr_set; */
  0,     /* long tp_dictoffset; */
  NULL,  /* initproc tp_init; */
  NULL,  /* allocfunc tp_alloc; */
  NULL,  // pyuni_object_new,       /* newfunc tp_new; */
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
PyObject *KPY_uni_types(void)
{
  PyObject *submodule = PyModule_Create(&kpy_types_module_def);
  KPy_TypesModule_State *state = (KPy_TypesModule_State *)PyModule_GetState(submodule);

  LUXO_kraken_luxo_pointer_create(&state->ptr);
  state->prop = LUXO_object_find_property(&state->ptr, "objects");

  /* Internal base types we have no other accessors for. */
  {
    static PyTypeObject *pyuni_types[] = {
      // &pyuni_object_meta_idprop_Type,
      &pyuni_object_Type,
      // &pyuni_prop_Type,
      // &pyuni_prop_array_Type,
      // &pyuni_prop_collection_Type,
      // &pyuni_func_Type,
    };

    PyObject *submodule_dict = PyModule_GetDict(submodule);
    for (int i = 0; i < TfArraySize(pyuni_types); i += 1) {
      PyDict_SetItemString(submodule_dict, pyuni_types[i]->tp_name, (PyObject *)pyuni_types[i]);
    }
  }

  return submodule;
}

KrakenPrim *pyuni_object_as_uni(PyObject *self, const bool parent, const char *error_prefix)
{
  KPy_KrakenPrim *py_uni = NULL;
  KrakenPrim *uni;

  /* Unfortunately PyObject_GetAttrString won't look up this types tp_dict first :/ */
  if (PyType_Check(self)) {
    py_uni = (KPy_KrakenPrim *)PyDict_GetItem(((PyTypeObject *)self)->tp_dict,
                                              kpy_intern_str_kr_uni);
    Py_XINCREF(py_uni);
  }

  if (parent) {
    /* be very careful with this since it will return a parent classes uni.
     * modifying this will do confusing stuff! */
    if (py_uni == NULL) {
      py_uni = (KPy_KrakenPrim *)PyObject_GetAttr(self, kpy_intern_str_kr_uni);
    }
  }

  if (py_uni == NULL) {
    PyErr_Format(PyExc_RuntimeError,
                 "%.200s, missing kr_uni attribute from '%.200s' instance (may not be registered)",
                 error_prefix,
                 Py_TYPE(self)->tp_name);
    return NULL;
  }

  if (!KPy_KrakenPrim_Check(py_uni)) {
    PyErr_Format(PyExc_TypeError,
                 "%.200s, kr_uni attribute wrong type '%.200s' on '%.200s'' instance",
                 error_prefix,
                 Py_TYPE(py_uni)->tp_name,
                 Py_TYPE(self)->tp_name);
    Py_DECREF(py_uni);
    return NULL;
  }

  if (py_uni->ptr.type != &LUXO_Object) {
    PyErr_Format(PyExc_TypeError,
                 "%.200s, kr_uni attribute not a LUXO_Object, on '%.200s'' instance",
                 error_prefix,
                 Py_TYPE(self)->tp_name);
    Py_DECREF(py_uni);
    return NULL;
  }

  uni = (KrakenPrim *)py_uni->ptr.data;
  Py_DECREF(py_uni);

  return uni;
}

PyDoc_STRVAR(pyuni_register_class_doc,
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
                                       pyuni_register_class,
                                       METH_O,
                                       pyuni_register_class_doc};
static PyObject *pyuni_register_class(PyObject *UNUSED(self), PyObject *py_class)
{
  kContext *C = nullptr;
  ReportList reports;
  ObjectRegisterFunc reg;
  KrakenPrim *uni;
  KrakenPrim *uni_new;
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

  if (PyDict_GetItem(((PyTypeObject *)py_class)->tp_dict, kpy_intern_str_kr_uni)) {
    PyErr_Format(PyExc_ValueError,
                 "register_class(...): "
                 "already registered as a subclass '%.200s'",
                 ((PyTypeObject *)py_class)->tp_name);
    return NULL;
  }

  if (!pyuni_write_check()) {
    PyErr_Format(PyExc_RuntimeError,
                 "register_class(...): "
                 "can't run in readonly state '%.200s'",
                 ((PyTypeObject *)py_class)->tp_name);
    return NULL;
  }

  /* Warning: gets parent classes uni, only for the register function. */
  uni = pyuni_object_as_uni(py_class, true, "register_class(...):");
  if (uni == NULL) {
    return NULL;
  }

  /* Check that we have a register callback for this type. */
  reg = LUXO_object_register(uni);

  if (!reg) {
    PyErr_Format(PyExc_ValueError,
                 "register_class(...): expected a subclass of a registerable "
                 "UNI type (%.200s does not support registration)",
                 LUXO_object_identifier(uni));
    return NULL;
  }

  /* Get the context, so register callback can do necessary refreshes. */
  C = KPY_context_get();

  /* Call the register callback with reports & identifier. */
  // KKE_reports_init(&reports, RPT_STORE);

  identifier = ((PyTypeObject *)py_class)->tp_name;

  uni_new = reg(CTX_data_main(C),
                &reports,
                py_class,
                identifier,
                kpy_class_validate,
                kpy_class_call,
                kpy_class_free);

  if (uni_new == NULL) {
    return NULL;
  }

  /* Takes a reference to 'py_class'. */
  // pyuni_subtype_set_rna(py_class, uni_new);

  /* Old uni still references us, keep the check in case registering somehow can free it. */
  // if (UNI_struct_py_type_get(uni)) {
  // UNI_struct_py_type_set(uni, NULL);
  // }

  // if (pyuni_deferred_register_class(uni_new, (PyTypeObject *)py_class) != 0) {
  // return NULL;
  // }

  /**
   * Call classed register method.
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

#ifdef USE_PYUNI_OBJECT_REFERENCE
static void pyuni_object_reference_set(KPy_KrakenPrim *self, PyObject *reference)
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
#endif /* !USE_PYRNA_STRUCT_REFERENCE */

#ifdef USE_PYUNI_INVALIDATE_WEAKREF
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
    weakinfo_hash = (RHash *)KKE_rhash_lookup(id_weakref_pool, id.GetAsToken());
  } else {
    /* First time, allocate pool. */
    // id_weakref_pool = KLI_rhash_ptr_new("uni_global_pool");
    // weakinfo_hash = NULL;
  }

  if (weakinfo_hash == NULL) {
    // weakinfo_hash = KKE_rhash_ptr_new("rna_id");
    KKE_rhash_insert(id_weakref_pool, id.GetAsToken(), weakinfo_hash);
  }

  return weakinfo_hash;
}

static void id_weakref_pool_add(const SdfPath &id, KPy_DummyPointerLUXO *pyuni)
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
  weakref = PyWeakref_NewRef((PyObject *)pyuni, weakref_cb_py);

  Py_DECREF(weakref_cb_py); /* Function owned by the weakref now. */

  /* Important to add at the end of the hash, since first removal looks at the end. */

  /* Using a hash table as a set, all 'id's are the same. */
  KKE_rhash_insert(weakinfo_hash, id.GetAsToken(), weakref);
  /* weakinfo_hash owns the weakref */
}
#endif /* USE_PYUNI_INVALIDATE_WEAKREF */

#ifdef USE_PYUNI_ITER

static void pyuni_prop_collection_iter_dealloc(KPy_CollectionPropertyLUXO *self);
static PyObject *pyuni_prop_collection_iter_next(KPy_CollectionPropertyLUXO *self);

static PyTypeObject pyuni_prop_collection_iter_Type = {
  PyVarObject_HEAD_INIT(NULL, 0) "kpy_prop_collection_iter", /* tp_name */
  sizeof(KPy_CollectionPropertyLUXO),                        /* tp_basicsize */
  0,                                                         /* tp_itemsize */
  /* methods */
  (destructor)pyuni_prop_collection_iter_dealloc, /* tp_dealloc */
  0,                                              /* tp_vectorcall_offset */
  NULL,                                           /* getattrfunc tp_getattr; */
  NULL,                                           /* setattrfunc tp_setattr; */
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
  PyObject_GenericGetAttr, /* getattrofunc tp_getattro; */
  NULL,                    /* setattrofunc tp_setattro; */

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
  /*** rich comparisons (subclassed) ***/
  NULL, /* richcmpfunc tp_richcompare; */

/***  weak reference enabler ***/
#  ifdef USE_WEAKREFS
  offsetof(KPy_CollectionPropertyLUXO, in_weakreflist), /* long tp_weaklistoffset; */
#  else
  0,
#  endif
  /*** Added in release 2.2 ***/
  /*   Iterators */
  PyObject_SelfIter,                             /* getiterfunc tp_iter; */
  (iternextfunc)pyuni_prop_collection_iter_next, /* iternextfunc tp_iternext; */

  /*** Attribute descriptor and subclassing stuff ***/
  NULL, /* struct PyMethodDef *tp_methods; */
  NULL, /* struct PyMemberDef *tp_members; */
  NULL, /* struct PyGetSetDef *tp_getset; */
  NULL, /* struct _typeobject *tp_base; */
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

static PyObject *pyuni_prop_collection_iter_CreatePyObject(PointerLUXO *ptr, PropertyLUXO *prop)
{
  KPy_CollectionPropertyLUXO *self = PyObject_New(KPy_CollectionPropertyLUXO,
                                                  &pyuni_prop_collection_iter_Type);

#  ifdef USE_WEAKREFS
  self->in_weakreflist = NULL;
#  endif

  LUXO_property_collection_begin(ptr, prop, self->iter);

  return (PyObject *)self;
}

static PyObject *pyuni_prop_collection_iter(KPy_PropertyLUXO *self)
{
  return pyuni_prop_collection_iter_CreatePyObject(&self->ptr, self->prop);
}

static PyObject *pyuni_prop_collection_iter_next(KPy_CollectionPropertyLUXO *self)
{
  if (self->iter.empty()) {
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
  }

  KPy_KrakenPrim *pyuni = (KPy_KrakenPrim *)pyuni_object_CreatePyObject(new KrakenPrim());

#  ifdef USE_PYUNI_OBJECT_REFERENCE
  if (pyuni) { /* Unlikely, but may fail. */
    if ((PyObject *)pyuni != Py_None) {
      /* hold a reference to the iterator since it may have
       * allocated memory 'pyuni' needs. eg: introspecting dynamic enum's. */
      /* TODO: we could have an api call to know if this is
       * needed since most collections don't */
      pyuni_object_reference_set(pyuni, (PyObject *)self);
    }
  }
#  endif /* !USE_PYUNI_OBJECT_REFERENCE */

  self->iter.push_back(new PropertyLUXO());

  return (PyObject *)pyuni;
}

static void pyuni_prop_collection_iter_dealloc(KPy_CollectionPropertyLUXO *self)
{
#  ifdef USE_WEAKREFS
  if (self->in_weakreflist != NULL) {
    PyObject_ClearWeakRefs((PyObject *)self);
  }
#  endif

  self->iter.end();

  PyObject_DEL(self);
}

#endif /* USE_PYUNI_ITER */

void KPY_uni_init(void)
{
  /* For some reason MSVC complains of these. */
  // #if defined(_MSC_VER)
  //   pyuni_object_meta_idprop_Type.tp_base = &PyType_Type;
  // #endif

  /* metaclass */
  // if (PyType_Ready(&pyuni_object_meta_idprop_Type) < 0) {
  //   return;
  // }

  if (PyType_Ready(&pyuni_object_Type) < 0) {
    return;
  }

  // if (PyType_Ready(&pyuni_prop_Type) < 0) {
  //   return;
  // }

  // if (PyType_Ready(&pyuni_prop_array_Type) < 0) {
  //   return;
  // }

  // if (PyType_Ready(&pyuni_prop_collection_Type) < 0) {
  //   return;
  // }

  // if (PyType_Ready(&pyuni_prop_collection_idprop_Type) < 0) {
  //   return;
  // }

  // if (PyType_Ready(&pyuni_func_Type) < 0) {
  //   return;
  // }

#ifdef USE_PYUNI_ITER
  if (PyType_Ready(&pyuni_prop_collection_iter_Type) < 0) {
    return;
  }
#endif
}


PyDoc_STRVAR(pyuni_unregister_class_doc,
             ".. method:: unregister_class(cls)\n"
             "\n"
             "   Unload the Python class from kraken.\n"
             "\n"
             "   If the class has an *unregister* class method it will be called\n"
             "   before unregistering.\n");
PyMethodDef meth_kpy_unregister_class = {
  "unregister_class",
  pyuni_unregister_class,
  METH_O,
  pyuni_unregister_class_doc,
};
static PyObject *pyuni_unregister_class(PyObject *UNUSED(self), PyObject *py_class)
{
  kContext *C = NULL;
  ObjectUnregisterFunc unreg;
  KrakenPrim *srna;
  PyObject *py_cls_meth;

  Py_RETURN_NONE;
}


/* 'kpy.data' from Python. */
static PointerLUXO *uni_module_ptr = NULL;
PyObject *KPY_uni_module(void)
{
  KPy_KrakenPrim *pyuni;
  PointerLUXO ptr;

  /* For now, return the base RNA type rather than a real module. */
  LUXO_main_pointer_create(G.main, &ptr);
  pyuni = (KPy_KrakenPrim *)pyuni_object_CreatePyObject(&ptr);

  uni_module_ptr = &pyuni->ptr;
  return (PyObject *)pyuni;
}

void KPY_update_uni_module(void)
{
  if (uni_module_ptr) {
    uni_module_ptr->data = G.main;
  }
}

void pyuni_alloc_types(void)
{
  // #ifdef DEBUG
  //   PyGILState_STATE gilstate;

  //   PointerLUXO ptr;
  //   PropertyLUXO *prop;

  //   gilstate = PyGILState_Ensure();

  //   /* Avoid doing this lookup for every getattr. */
  //   LUXO_kraken_luxo_pointer_create(&ptr);
  //   prop = LUXO_object_find_property(&ptr, "objects");

  //   UNI_PROP_BEGIN (&ptr, itemptr, prop) {
  //     PyObject *item = pyuni_object_Subtype(&itemptr);
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
  //   UNI_PROP_END;

  //   PyGILState_Release(gilstate);
  // #endif /* DEBUG */
}

/*-----------------------CreatePyObject---------------------------------*/
PyObject *pyuni_object_CreatePyObject(PointerLUXO *ptr)
{
  KPy_KrakenPrim *pyuni = NULL;

  /* Note: don't rely on this to return None since NULL data with a valid type can often crash. */
  if (ptr->data == NULL && ptr->type == NULL) { /* Operator RNA has NULL data. */
    Py_RETURN_NONE;
  }

  void **instance = ptr->data ? LUXO_object_instance(ptr) : NULL;
  if (instance && *instance) {
    pyuni = (KPy_KrakenPrim *)*instance;

    /* Refine may have changed types after the first instance was created. */
    if (ptr->type == pyuni->ptr.type) {
      Py_INCREF(pyuni);
      return (PyObject *)pyuni;
    }

    /* Existing users will need to use 'type_recast' method. */
    Py_DECREF(pyuni);
    *instance = NULL;
    /* Continue as if no instance was made. */
#if 0 /* No need to assign, will be written to next... */
      pyuni = NULL;
#endif
  }

  // {
  // PyTypeObject *tp = (PyTypeObject *)pyuni_object_Subtype(ptr);

  // if (tp) {
  // pyuni = (KPy_KrakenPrim *)tp->tp_alloc(tp, 0);
  // #ifdef USE_PYUNI_OBJECT_REFERENCE
  /* #PyType_GenericAlloc will have set tracking.
   * We only want tracking when `StructRNA.reference` has been set. */
  // if (pyuni != NULL) {
  // PyObject_GC_UnTrack(pyuni);
  // }
  // #endif
  // Py_DECREF(tp); /* srna owns, can't hold a reference. */
  // }
  // else {
  // TF_WARN("could not make type '%s'", LUXO_object_identifier(ptr->type));

#ifdef USE_PYUNI_OBJECT_REFERENCE
  pyuni = (KPy_KrakenPrim *)PyObject_GC_New(KPy_KrakenPrim, &pyuni_object_Type);
#else
  pyuni = (KPy_KrakenPrim *)PyObject_New(KPy_KrakenPrim, &pyuni_object_Type);
#endif

  // #ifdef USE_WEAKREFS
  // if (pyuni != NULL) {
  // pyuni->in_weakreflist = NULL;
  // }
  // #endif
  // }
  // }

  if (pyuni == NULL) {
    PyErr_SetString(PyExc_MemoryError, "couldn't create kpy_object");
    return NULL;
  }

  /* Blender's instance owns a reference (to avoid Python freeing it). */
  if (instance) {
    *instance = pyuni;
    Py_INCREF(pyuni);
  }

  pyuni->ptr = *ptr;
#ifdef PYUNI_FREE_SUPPORT
  pyuni->freeptr = false;
#endif

#ifdef USE_PYUNI_OBJECT_REFERENCE
  pyuni->reference = NULL;
#endif

  // PyC_ObSpit("NewStructRNA: ", (PyObject *)pyuni);

#ifdef USE_PYUNI_INVALIDATE_WEAKREF
  if (!ptr->path.IsEmpty()) {
    id_weakref_pool_add(ptr->path, (KPy_DummyPointerLUXO *)pyuni);
  }
#endif
  return (PyObject *)pyuni;
}

static PyObject *pyuni_kr_owner_id_get(PyObject *UNUSED(self))
{
  // const char *name = RNA_struct_state_owner_get();
  // if (name) {
  //   return PyUnicode_FromString(name);
  // }
  Py_RETURN_NONE;
}

static PyObject *pyuni_kr_owner_id_set(PyObject *UNUSED(self), PyObject *value)
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
  // RNA_struct_state_owner_set(name);
  Py_RETURN_NONE;
}

PyMethodDef meth_kpy_owner_id_get = {
  "_kr_owner_id_get",
  (PyCFunction)pyuni_kr_owner_id_get,
  METH_NOARGS,
  NULL,
};
PyMethodDef meth_kpy_owner_id_set = {
  "_kr_owner_id_set",
  (PyCFunction)pyuni_kr_owner_id_set,
  METH_O,
  NULL,
};

WABI_NAMESPACE_END