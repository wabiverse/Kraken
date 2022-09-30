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

#include <float.h> /* FLT_MIN/MAX */
#include <stddef.h>

#include "KLI_utildefines.h"
#include "KLI_string.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_robinhood.h"
#include "KKE_utils.h"
#include "KKE_report.h"

#include "LUXO_access.h"
#include "LUXO_runtime.h"

#include "USD_wm_types.h"
#include "USD_types.h"
#include "USD_object.h"

#include "kpy_utildefines.h"
#include "kpy_interface.h"
#include "kpy_intern_string.h"
#include "kpy_stage.h"
#include "kpy_capi_utils.h"

#include <wabi/usd/usd/prim.h>
#include <wabi/base/tf/iterator.h>

#define USE_PEDANTIC_WRITE
#define USE_MATHUTILS
#define USE_STRING_COERCE

#define USE_POSTPONED_ANNOTATIONS

#include <boost/python.hpp>
#include <boost/python/overloads.hpp>

using namespace boost::python;

WABI_NAMESPACE_USING

KRAKEN_NAMESPACE_USING

KPy_KrakenStage *kpy_context_module = NULL; /* for fast access */

// PyTypeObject pystage_struct_meta_idprop_Type;
// PyTypeObject pystage_struct_Type;
// PyTypeObject pystage_prop_Type;
// PyTypeObject pystage_prop_array_Type;
// PyTypeObject pystage_prop_collection_Type;
// PyTypeObject pystage_func_Type;

static PyObject *pystage_register_class(PyObject *self, PyObject *py_class);
static PyObject *pystage_unregister_class(PyObject *self, PyObject *py_class);

static bool uni_disallow_writes = false;

bool pystage_write_check(void)
{
  return !uni_disallow_writes;
}

void pystage_write_set(bool val)
{
  uni_disallow_writes = !val;
}

static int kpy_class_validate(const UsdPrim &type, void *py_data, int *have_function)
{
  // return kpy_class_validate_recursive(dummyptr, dummyptr->type, py_data, have_function);
  return 1;
}

static int kpy_class_call(kContext *C, const UsdPrim &type, void *func, UsdPropertyVector parms)
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

PyTypeObject pystage_struct_meta_idprop_Type = {
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
  NULL,                                                          /* hashfunc tp_hash; */
  NULL,                                                          /* ternaryfunc tp_call; */
  NULL,                                                          /* reprfunc tp_str; */
  NULL /* (getattrofunc) pystage_struct_meta_idprop_getattro */, /* getattrofunc tp_getattro; */

  NULL,  // (setattrofunc)pystage_struct_meta_idprop_setattro,             /* setattrofunc
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

PyTypeObject pystage_struct_Type = {
  PyVarObject_HEAD_INIT(NULL, 0) "kpy_struct", /* tp_name */
  sizeof(KPy_KrakenStage),                     /* tp_basicsize */
  0,                                           /* tp_itemsize */
  /* methods */
  NULL,  //(destructor)pystage_struct_dealloc, /* tp_dealloc */
  0,     /* tp_vectorcall_offset */
  NULL,  /* getattrfunc tp_getattr; */
  NULL,  /* setattrfunc tp_setattr; */
  NULL,
  /* tp_compare */ /* DEPRECATED in Python 3.0! */
  NULL,            //(reprfunc)pystage_struct_repr, /* tp_repr */

  /* Method suites for standard classes */

  NULL,  /* PyNumberMethods *tp_as_number; */
  NULL,  //&pystage_struct_as_sequence, /* PySequenceMethods *tp_as_sequence; */
  NULL,  //&pystage_struct_as_mapping,  /* PyMappingMethods *tp_as_mapping; */

  /* More standard operations (here for binary compatibility) */

  NULL,  //(hashfunc)pystage_struct_hash,         /* hashfunc tp_hash; */
  NULL,  /* ternaryfunc tp_call; */
  NULL,  //(reprfunc)pystage_struct_str,          /* reprfunc tp_str; */
  NULL,  //(getattrofunc)pystage_struct_getattro, /* getattrofunc tp_getattro; */
  NULL,  //(setattrofunc)pystage_struct_setattro, /* setattrofunc tp_setattro; */

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
  NULL,  //(traverseproc)pystage_struct_traverse, /* traverseproc tp_traverse; */

  /* delete references to contained objects */
  NULL,  //(inquiry)pystage_struct_clear, /* inquiry tp_clear; */
#else
  NULL,         /* traverseproc tp_traverse; */

  /* delete references to contained objects */
  NULL, /* inquiry tp_clear; */
#endif /* !USE_PYRNA_STRUCT_REFERENCE */

  /***  Assigned meaning in release 2.1 ***/
  /*** rich comparisons ***/
  NULL,  //(richcmpfunc)pystage_struct_richcmp, /* richcmpfunc tp_richcompare; */

/***  weak reference enabler ***/
#ifdef USE_WEAKREFS
  offsetof(KPy_KrakenStage, in_weakreflist), /* long tp_weaklistoffset; */
#else
  0,
#endif
  /*** Added in release 2.2 ***/
  /*   Iterators */
  NULL, /* getiterfunc tp_iter; */
  NULL, /* iternextfunc tp_iternext; */

  /*** Attribute descriptor and subclassing stuff ***/
  NULL,  // pystage_struct_methods,   /* struct PyMethodDef *tp_methods; */
  NULL,  /* struct PyMemberDef *tp_members; */
  NULL,  // pystage_struct_getseters, /* struct PyGetSetDef *tp_getset; */
  NULL,  /* struct _typeobject *tp_base; */
  NULL,  /* PyObject *tp_dict; */
  NULL,  /* descrgetfunc tp_descr_get; */
  NULL,  /* descrsetfunc tp_descr_set; */
  0,     /* long tp_dictoffset; */
  NULL,  /* initproc tp_init; */
  NULL,  /* allocfunc tp_alloc; */
  NULL,  // pystage_struct_new,       /* newfunc tp_new; */
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

/* Check if we have a native Python subclass, use it when it exists
 * return a borrowed reference. */
static PyObject *kpy_types_dict = NULL;

static PyObject *pystage_srna_ExternalType(KrakenPRIM *srna)
{
  const char *idname = LUXO_struct_identifier(srna);
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
    PyObject *base_compare = pystage_srna_PyBase(srna);
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
static PyObject *pystage_srna_Subtype(KrakenPRIM *srna)
{
  PyObject *newclass = NULL;

  /* Stupid/simple case. */
  if (srna == NULL) {
    newclass = NULL; /* Nothing to do. */
  }                  /* The class may have already been declared & allocated. */
  else if ((newclass = (PyObject *)LUXO_struct_py_type_get(srna))) {
    Py_INCREF(newclass);
  } /* Check if kpy_types.py module has the class defined in it. */
  else if ((newclass = pystage_srna_ExternalType(srna))) {
    pystage_subtype_set_rna(newclass, srna);
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

    /* Assume RNA_struct_py_type_get(srna) was already checked. */
    PyObject *py_base = pystage_srna_PyBase(srna);
    PyObject *metaclass;
    const char *idname = LUXO_struct_identifier(srna);

    /* Remove `__doc__` for now because we don't need it to generate docs. */
#if 0
    const char *descr = RNA_struct_ui_description(srna);
    if (!descr) {
      descr = "(no docs)";
    }
#endif

    if (!PyObject_IsSubclass(py_base, (PyObject *)&pystage_struct_meta_idprop_Type)) {
      metaclass = (PyObject *)&pystage_struct_meta_idprop_Type;
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
      /* srna owns one, and the other is owned by the caller. */
      pystage_subtype_set_rna(newclass, srna);

      /* XXX, adding this back segfaults Blender on load. */
      // Py_DECREF(newclass); /* let srna own */
    } else {
      /* This should not happen. */
      TF_WARN("failed to register '%s'", idname);
      PyErr_Print();
      PyErr_Clear();
    }
  }

  return newclass;
}

KrakenPRIM *LUXO_struct_base(KrakenPRIM *type)
{
  return type->base;
}

/* polymorphism. */
PyObject *pystage_srna_PyBase(KrakenPRIM *srna)
{
  PyObject *py_base = NULL;

  /* Get the base type. */
  KrakenPRIM *base = LUXO_struct_base(srna);

  if (base && base != srna) {
    py_base = pystage_srna_Subtype(base);
    Py_DECREF(py_base);
  }

  if (py_base == NULL) {
    py_base = (PyObject *)&pystage_struct_Type;
  }

  return py_base;
}

static PyObject *pystage_struct_Subtype(KrakenPRIM *ptr)
{
  return pystage_srna_Subtype(srna_from_ptr(ptr));
}

int LUXO_property_collection_lookup_token_index(KrakenPRIM *ptr,
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

int LUXO_property_collection_lookup_token(KrakenPRIM *ptr, const TfToken &key, KrakenPRIM *r_ptr)
{
  int index;
  return LUXO_property_collection_lookup_token_index(ptr, key, r_ptr, &index);
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
  } else if (LUXO_property_collection_lookup_token(&state->ptr, TfToken(name), &newptr)) {
    ret = pystage_struct_Subtype(&newptr);
    if (ret == NULL) {
      PyErr_Format(PyExc_RuntimeError,
                   "kpy.types.%.200s subtype could not be generated, this is a bug!",
                   PyUnicode_AsUTF8(pyname));
    }
  } else {
#if 0
    PyErr_Format(PyExc_AttributeError,
                 "kpy.types.%.200s RNA_Struct does not exist",
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
    PyList_APPEND(ret, PyUnicode_FromString(LUXO_struct_identifier(&prim)));
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
PyObject *KPY_uni_types(void)
{
  PyObject *submodule = PyModule_Create(&kpy_types_module_def);
  KPy_TypesModule_State *state = (KPy_TypesModule_State *)PyModule_GetState(submodule);

  LUXO_kraken_luxo_pointer_create(&state->ptr);
  LUXO_object_find_property(&state->ptr, TfToken("collection:structs:includes"), &state->prop);

  /* Internal base types we have no other accessors for. */
  {
    static PyTypeObject *pystage_types[] = {
      &pystage_struct_meta_idprop_Type,
      &pystage_struct_Type,
      // &pystage_prop_Type,
      // &pystage_prop_array_Type,
      // &pystage_prop_collection_Type,
      // &pystage_func_Type,
    };

    PyObject *submodule_dict = PyModule_GetDict(submodule);
    for (int i = 0; i < ARRAY_SIZE(pystage_types); i += 1) {
      PyDict_SetItemString(submodule_dict,
                           pystage_types[i]->tp_name,
                           (PyObject *)pystage_types[i]);
    }
  }

  return submodule;
}

// static PyObject *pystage_func_to_py(const KrakenPRIM *ptr, KrakenPRIM *func)
// {
//   // KPy_KrakenFUNC *pyfunc = (KPy_KrakenFUNC *)PyObject_NEW(KPy_KrakenFUNC,
//   // &pystage_func_Type); pyfunc->ptr = *ptr; pyfunc->func = (KrakenFUNC *)func; return
//   (PyObject
//   // *)pyfunc;
//   Py_RETURN_NONE;
// }

void pystage_subtype_set_rna(PyObject *newclass, KrakenPRIM *srna)
{
  KrakenPRIM ptr;
  PyObject *item;

  Py_INCREF(newclass);

  if (LUXO_struct_py_type_get(srna)) {
    PyC_ObSpit("LUXO WAS SET - ", (PyObject *)LUXO_struct_py_type_get(srna));
  }

  Py_XDECREF(((PyObject *)LUXO_struct_py_type_get(srna)));

  LUXO_struct_py_type_set(srna, (void *)newclass); /* Store for later use */

  /* Not 100% needed, but useful,
   * having an instance within a type looks wrong, but this instance _is_ a LUXO type. */

  /* Python deals with the circular reference. */
  LUXO_pointer_create(NULL, &LUXO_Struct, srna, &ptr);
  item = pystage_struct_CreatePyObject(&ptr);

  /* NOTE: must set the class not the __dict__ else the internal slots are not updated correctly.
   */
  PyObject_SetAttr(newclass, kpy_intern_str_kr_stage, item);
  Py_DECREF(item);

  /* Add staticmethods and classmethods. */
  // {
  // const KrakenPRIM func_ptr(*srna);

  // auto &lb = LUXO_struct_type_functions(srna);
  // for (auto link : lb) {
  //   KrakenFUNC *func = (KrakenFUNC *)link;
  //   const int flag = LUXO_function_flag(func);
  //   if ((flag & FUNC_NO_SELF) &&         /* Is staticmethod or classmethod. */
  //       (flag & FUNC_REGISTER) == false) /* Is not for registration. */
  //   {
  //     PyObject *func_py = pystage_func_to_py(&func_ptr, (KrakenPRIM *)func);
  //     PyObject_SetAttrString(newclass, LUXO_function_identifier(func), func_py);
  //     Py_DECREF(func_py);
  //   }
  // }
  // }

  /* Done with LUXO instance. */
}

KrakenPRIM *pystage_struct_as_srna(PyObject *self, const bool parent, const char *error_prefix)
{
  KPy_KrakenStage *py_srna = NULL;
  KrakenPRIM *srna;

  /* Unfortunately PyObject_GetAttrString won't look up this types tp_dict first :/ */
  if (PyType_Check(self)) {
    py_srna = (KPy_KrakenStage *)PyDict_GetItem(((PyTypeObject *)self)->tp_dict,
                                                kpy_intern_str_kr_stage);
    Py_XINCREF(py_srna);
  }

  if (parent) {
    /* be very careful with this since it will return a parent classes srna.
     * modifying this will do confusing stuff! */
    if (py_srna == NULL) {
      py_srna = (KPy_KrakenStage *)PyObject_GetAttr(self, kpy_intern_str_kr_stage);
    }
  }

  if (py_srna == NULL) {
    PyErr_Format(PyExc_RuntimeError,
                 "%.200s, missing bl_rna attribute from '%.200s' instance (may not be registered)",
                 error_prefix,
                 Py_TYPE(self)->tp_name);
    return NULL;
  }

  if (!KPy_KrakenStage_Check(py_srna)) {
    PyErr_Format(PyExc_TypeError,
                 "%.200s, bl_rna attribute wrong type '%.200s' on '%.200s'' instance",
                 error_prefix,
                 Py_TYPE(py_srna)->tp_name,
                 Py_TYPE(self)->tp_name);
    Py_DECREF(py_srna);
    return NULL;
  }

  if (!py_srna->ptr->GetPseudoRoot().IsValid()) {
    PyErr_Format(PyExc_TypeError,
                 "%.200s, bl_rna attribute not a RNA_Struct, on '%.200s'' instance",
                 error_prefix,
                 Py_TYPE(self)->tp_name);
    Py_DECREF(py_srna);
    return NULL;
  }

  srna = new KrakenPRIM(py_srna->ptr->GetPseudoRoot());
  Py_DECREF(py_srna);

  return srna;
}


static int deferred_register_prop(KrakenPRIM *srna, PyObject *key, PyObject *item)
{
  return 0;
}

/**
 * Extract `__annotations__` using `typing.get_type_hints` which handles the delayed evaluation.
 */
static int pystage_deferred_register_class_from_type_hints(KrakenPRIM *srna,
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
        ret = deferred_register_prop(srna, key, item);
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

static int pystage_deferred_register_props(KrakenPRIM *srna, PyObject *class_dict)
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
      ret = deferred_register_prop(srna, key, item);

      if (ret != 0) {
        break;
      }
    }
  }

  return ret;
}

static int pystage_deferred_register_class_recursive(KrakenPRIM *srna, PyTypeObject *py_class)
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
     * So only scan base classes which are not subclasses if blender types.
     * This best fits having 'mix-in' classes for operators and render engines.
     */
    if (py_superclass != &PyBaseObject_Type &&
        !PyObject_IsSubclass((PyObject *)py_superclass, (PyObject *)&pystage_struct_Type)) {
      ret = pystage_deferred_register_class_recursive(srna, py_superclass);

      if (ret != 0) {
        return ret;
      }
    }
  }

  /* Not register out own properties. */
  /* getattr(..., "__dict__") returns a proxy. */
  return pystage_deferred_register_props(srna, py_class->tp_dict);
}

int pystage_deferred_register_class(KrakenPRIM *srna, PyTypeObject *py_class)
{
  /* Panels and Menus don't need this
   * save some time and skip the checks here */
  // if (!LUXO_struct_idprops_register_check(srna)) {
  //   return 0;
  // }

#ifdef USE_POSTPONED_ANNOTATIONS
  const bool use_postponed_annotations = true;
#else
  const bool use_postponed_annotations = false;
#endif

  if (use_postponed_annotations) {
    return pystage_deferred_register_class_from_type_hints(srna, py_class);
  }
  return pystage_deferred_register_class_recursive(srna, py_class);
}


PyDoc_STRVAR(pystage_register_class_doc,
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
                                       pystage_register_class,
                                       METH_O,
                                       pystage_register_class_doc};
static PyObject *pystage_register_class(PyObject *UNUSED(self), PyObject *py_class)
{
  kContext *C = NULL;
  ReportList reports;
  ObjectRegisterFunc reg;
  KrakenPRIM *srna;
  KrakenPRIM *srna_new;
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

  if (PyDict_GetItem(((PyTypeObject *)py_class)->tp_dict, kpy_intern_str_kr_stage)) {
    PyErr_Format(PyExc_ValueError,
                 "register_class(...): "
                 "already registered as a subclass '%.200s'",
                 ((PyTypeObject *)py_class)->tp_name);
    return NULL;
  }

  if (!pystage_write_check()) {
    PyErr_Format(PyExc_RuntimeError,
                 "register_class(...): "
                 "can't run in readonly state '%.200s'",
                 ((PyTypeObject *)py_class)->tp_name);
    return NULL;
  }

  /* WARNING: gets parent classes srna, only for the register function. */
  srna = pystage_struct_as_srna(py_class, true, "register_class(...):");
  if (srna == NULL) {
    return NULL;
  }

  /* Fails in some cases, so can't use this check, but would like to :| */
#if 0
  if (RNA_struct_py_type_get(srna)) {
    PyErr_Format(PyExc_ValueError,
                 "register_class(...): %.200s's parent class %.200s is already registered, this "
                 "is not allowed",
                 ((PyTypeObject *)py_class)->tp_name,
                 RNA_struct_identifier(srna));
    return NULL;
  }
#endif

  /* Check that we have a register callback for this type. */
  reg = LUXO_struct_register(srna);

  if (!reg) {
    PyErr_Format(PyExc_ValueError,
                 "register_class(...): expected a subclass of a registerable "
                 "LUXO type (%.200s does not support registration)",
                 LUXO_struct_identifier(srna));
    return NULL;
  }

  /* Get the context, so register callback can do necessary refreshes. */
  C = KPY_context_get();

  /* Call the register callback with reports & identifier. */
  KKE_reports_init(&reports, RPT_STORE);

  identifier = ((PyTypeObject *)py_class)->tp_name;

  srna_new = reg(CTX_data_main(C),
                 &reports,
                 py_class,
                 identifier,
                 kpy_class_validate,
                 kpy_class_call,
                 kpy_class_free);

  if (!reports.list.empty()) {
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
  if (srna_new == NULL) {
    return NULL;
  }

  /* Takes a reference to 'py_class'. */
  pystage_subtype_set_rna(py_class, srna_new);

  /* Old srna still references us, keep the check in case registering somehow can free it. */
  if (LUXO_struct_py_type_get(srna)) {
    LUXO_struct_py_type_set(srna, NULL);
#if 0
    /* Should be able to do this XXX since the old RNA adds a new ref. */
    Py_DECREF(py_class);
#endif
  }

  /* Can't use this because it returns a dict proxy
   *
   * item = PyObject_GetAttrString(py_class, "__dict__");
   */
  if (pystage_deferred_register_class(srna_new, (PyTypeObject *)py_class) != 0) {
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
static void pystage_struct_reference_set(KPy_KrakenStage *self, PyObject *reference)
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

static void id_weakref_pool_add(const SdfPath &id, KPy_DummyKrakenPRIM *pystage)
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
  weakref = PyWeakref_NewRef((PyObject *)pystage, weakref_cb_py);

  Py_DECREF(weakref_cb_py); /* Function owned by the weakref now. */

  /* Important to add at the end of the hash, since first removal looks at the end. */

  /* Using a hash table as a set, all 'id's are the same. */
  KKE_rhash_insert(weakinfo_hash, id.GetAsToken(), weakref);
  /* weakinfo_hash owns the weakref */
}
#endif /* USE_PYUSD_INVALIDATE_WEAKREF */

void KPY_uni_init(void)
{
/* For some reason MSVC complains of these. */
#if defined(_MSC_VER)
  pystage_struct_meta_idprop_Type.tp_base = &PyType_Type;
#endif

  /* metaclass */
  if (PyType_Ready(&pystage_struct_meta_idprop_Type) < 0) {
    return;
  }

  if (PyType_Ready(&pystage_struct_Type) < 0) {
    return;
  }

  // if (PyType_Ready(&pystage_prop_Type) < 0) {
  //   return;
  // }

  // if (PyType_Ready(&pystage_prop_array_Type) < 0) {
  //   return;
  // }

  // if (PyType_Ready(&pystage_prop_collection_Type) < 0) {
  //   return;
  // }

  // if (PyType_Ready(&pystage_prop_collection_idprop_Type) < 0) {
  //   return;
  // }

  // if (PyType_Ready(&pystage_func_Type) < 0) {
  //   return;
  // }

#ifdef USE_PYUSD_ITER
  // if (PyType_Ready(&pystage_prop_collection_iter_Type) < 0) {
  //   return;
  // }
#endif
}


PyDoc_STRVAR(pystage_unregister_class_doc,
             ".. method:: unregister_class(cls)\n"
             "\n"
             "   Unload the Python class from kraken.\n"
             "\n"
             "   If the class has an *unregister* class method it will be called\n"
             "   before unregistering.\n");
PyMethodDef meth_kpy_unregister_class = {
  "unregister_class",
  pystage_unregister_class,
  METH_O,
  pystage_unregister_class_doc,
};
static PyObject *pystage_unregister_class(PyObject *UNUSED(self), PyObject *py_class)
{
  kContext *C = NULL;
  ObjectUnregisterFunc unreg;
  UsdPrim srna;
  PyObject *py_cls_meth;

  Py_RETURN_NONE;
}


/* 'kpy.data' from Python. */
static UsdStageWeakPtr stage_module_ptr = TfNullPtr;
PyObject *KPY_stage_module(void)
{
  KPy_KrakenStage *pystage;
  KrakenPRIM ptr;

  /* For now, return the base RNA type rather than a real module. */
  LUXO_main_pointer_create(G.main, &ptr);
  pystage = (KPy_KrakenStage *)pystage_struct_CreatePyObject(&ptr);

  stage_module_ptr = pystage->ptr;
  return (PyObject *)pystage;
}

void KPY_update_stage_module(void)
{
  if (stage_module_ptr) {
    stage_module_ptr->Open(G.main->stage_id);
  }
}

void pystage_alloc_types(void)
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
  //     PyObject *item = pystage_struct_Subtype(&itemptr);
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
PyObject *pystage_struct_CreatePyObject(KrakenPRIM *ptr)
{
  KPy_KrakenStage *pystage = NULL;

  /* NOTE: don't rely on this to return None since NULL data with a valid type can often crash. */
  if (ptr->data == NULL && ptr->type == NULL) { /* Operator RNA has NULL data. */
    Py_RETURN_NONE;
  }

  void **instance = ptr->data ? LUXO_struct_instance(ptr) : NULL;
  if (instance && *instance) {
    pystage = (KPy_KrakenStage *)*instance;

    /* Refine may have changed types after the first instance was created. */
    if (ptr->type->GetPrim() == pystage->ptr->GetPrimAtPath(ptr->type->GetPath())) {
      Py_INCREF(pystage);
      return (PyObject *)pystage;
    }

    /* Existing users will need to use 'type_recast' method. */
    Py_DECREF(pystage);
    *instance = NULL;
    /* Continue as if no instance was made. */
#if 0 /* No need to assign, will be written to next... */
      pystage = NULL;
#endif
  }

  {
    PyTypeObject *tp = (PyTypeObject *)pystage_struct_Subtype(ptr);

    if (tp) {
      pystage = (KPy_KrakenStage *)tp->tp_alloc(tp, 0);
#ifdef USE_PYRNA_STRUCT_REFERENCE
      /* #PyType_GenericAlloc will have set tracking.
       * We only want tracking when `KrakenSTAGE.reference` has been set. */
      if (pystage != NULL) {
        PyObject_GC_UnTrack(pystage);
      }
#endif
      Py_DECREF(tp); /* srna owns, can't hold a reference. */
    } else {
      TF_WARN("kpy: could not make type '%s'", LUXO_struct_identifier(ptr));

#ifdef USE_PYRNA_STRUCT_REFERENCE
      pystage = (KPyKPy_KrakenStage_StructLUXO *)PyObject_GC_New(KPy_KrakenStage,
                                                                 &pystage_struct_Type);
#else
      pystage = (KPy_KrakenStage *)PyObject_New(KPy_KrakenStage, &pystage_struct_Type);
#endif

#ifdef USE_WEAKREFS
      if (pystage != NULL) {
        pystage->in_weakreflist = NULL;
      }
#endif
    }
  }

  if (pystage == NULL) {
    PyErr_SetString(PyExc_MemoryError, "couldn't create kpy_struct object");
    return NULL;
  }

  /* Blender's instance owns a reference (to avoid Python freeing it). */
  if (instance) {
    *instance = pystage;
    Py_INCREF(pystage);
  }

  pystage->ptr = ptr->GetStage();
#ifdef PYRNA_FREE_SUPPORT
  pystage->freeptr = false;
#endif

#ifdef USE_PYRNA_STRUCT_REFERENCE
  pystage->reference = NULL;
#endif

  // PyC_ObSpit("NewKrakenSTAGE: ", (PyObject *)pystage);

#ifdef USE_PYRNA_INVALIDATE_WEAKREF
  if (ptr->owner_id) {
    id_weakref_pool_add(ptr->owner_id, (KPy_DummyKrakenPRIM *)pystage);
  }
#endif
  return (PyObject *)pystage;
}

static PyObject *pystage_kr_owner_id_get(PyObject *UNUSED(self))
{
  // const char *name = RNA_struct_state_owner_get();
  // if (name) {
  //   return PyUnicode_FromString(name);
  // }
  Py_RETURN_NONE;
}

static PyObject *pystage_kr_owner_id_set(PyObject *UNUSED(self), PyObject *value)
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
  (PyCFunction)pystage_kr_owner_id_get,
  METH_NOARGS,
  NULL,
};
PyMethodDef meth_kpy_owner_id_set = {
  "_kr_owner_id_set",
  (PyCFunction)pystage_kr_owner_id_set,
  METH_O,
  NULL,
};
