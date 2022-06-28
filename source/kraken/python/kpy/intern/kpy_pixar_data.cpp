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

#include "KLI_utildefines.h"
#include "KLI_string_utils.h"

#include "KKE_appdir.h"
#include "KKE_version.h"
#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_robinhood.h"
#include "KKE_utils.h"

#include "LUXO_access.h"
#include "LUXO_runtime.h"

#include "UNI_factory.h"
#include "UNI_types.h"
#include "UNI_wm_types.h"

#include "kpy.h"
#include "kpy_app.h"
#include "kpy_capi_utils.h"
#include "kpy_path.h"
#include "kpy_interface.h"
#include "kpy_intern_string.h"
#include "kpy_library.h"
#include "kpy_stage.h"
#include "kpy_pixar_data.h"

#include "wabi_python.h"

WABI_NAMESPACE_BEGIN

struct KPy_DataContext
{
  PyObject_HEAD /* Required Python macro. */
    KPy_StructLUXO *data_luxo;
  char filepath[1024];
};

static PyObject *kpy_pixar_data_temp_data(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *kpy_pixar_data_context_enter(KPy_DataContext *self);
static PyObject *kpy_pixar_data_context_exit(KPy_DataContext *self, PyObject *args);

static PyMethodDef kpy_pixar_data_context_methods[] = {
  {"__enter__",  (PyCFunction)kpy_pixar_data_context_enter, METH_NOARGS},
  {"__exit__", (PyCFunction)kpy_pixar_data_context_exit, METH_VARARGS},
  {NULL  }  /* sentinel */
};

static int kpy_pixar_data_context_traverse(KPy_DataContext *self, visitproc visit, void *arg)
{
  Py_VISIT(self->data_luxo);
  return 0;
}

static int kpy_pixar_data_context_clear(KPy_DataContext *self)
{
  Py_CLEAR(self->data_luxo);
  return 0;
}

static void kpy_pixar_data_context_dealloc(KPy_DataContext *self)
{
  PyObject_GC_UnTrack(self);
  Py_CLEAR(self->data_luxo);
  PyObject_GC_Del(self);
}


static PyTypeObject kpy_pixar_data_context_Type = {
  PyVarObject_HEAD_INIT(NULL, 0) "kpy_pixar_data_context", /* tp_name */
  sizeof(KPy_DataContext),                                 /* tp_basicsize */
  0,                                                       /* tp_itemsize */
  /* methods */
  (destructor)kpy_pixar_data_context_dealloc, /* tp_dealloc */
  0,                                          /* tp_vectorcall_offset */
  NULL,                                       /* getattrfunc tp_getattr; */
  NULL,                                       /* setattrfunc tp_setattr; */
  NULL,
  /* tp_compare */ /* DEPRECATED in python 3.0! */
  NULL,            /* tp_repr */

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
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, /* long tp_flags; */

  NULL, /*  char *tp_doc;  Documentation string */
  /*** Assigned meaning in release 2.0 ***/
  /* call function for all accessible objects */
  (traverseproc)kpy_pixar_data_context_traverse, /* traverseproc tp_traverse; */

  /* delete references to contained objects */
  (inquiry)kpy_pixar_data_context_clear, /* inquiry tp_clear; */

  /***  Assigned meaning in release 2.1 ***/
  /*** rich comparisons (subclassed) ***/
  NULL, /* richcmpfunc tp_richcompare; */

  /***  weak reference enabler ***/
  0,
  /*** Added in release 2.2 ***/
  /*   Iterators */
  NULL, /* getiterfunc tp_iter; */
  NULL, /* iternextfunc tp_iternext; */

  /*** Attribute descriptor and subclassing stuff ***/
  kpy_pixar_data_context_methods, /* struct PyMethodDef *tp_methods; */
  NULL,                           /* struct PyMemberDef *tp_members; */
  NULL,                           /* struct PyGetSetDef *tp_getset; */
  NULL,                           /* struct _typeobject *tp_base; */
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

PyDoc_STRVAR(kpy_pixar_data_context_load_doc,
             ".. method:: temp_data(filepath=None)\n"
             "\n"
             "   A context manager that temporarily creates blender file data.\n"
             "\n"
             "   :arg filepath: The file path for the newly temporary data. "
             "When None, the path of the currently open file is used.\n"
             "   :type filepath: str or NoneType\n"
             "\n"
             "   :return: Blend file data which is freed once the context exists.\n"
             "   :rtype: :class:`kpy.types.BlendData`\n");

static PyObject *kpy_pixar_data_temp_data(PyObject *UNUSED(self), PyObject *args, PyObject *kw)
{
  KPy_DataContext *ret;
  const char *filepath = NULL;
  static const char *_keywords[] = {"filepath", NULL};
  static _PyArg_Parser _parser = {
    "|$" /* Optional keyword only arguments. */
    "z"  /* `filepath` */
    ":temp_data",
    _keywords,
    0,
  };
  if (!_PyArg_ParseTupleAndKeywordsFast(args, kw, &_parser, &filepath)) {
    return NULL;
  }

  ret = PyObject_GC_New(KPy_DataContext, &kpy_pixar_data_context_Type);

  STRNCPY(ret->filepath, filepath ? filepath : G.filepath);

  return (PyObject *)ret;
}

static PyObject *kpy_pixar_data_context_enter(KPy_DataContext *self)
{
  Main *kmain_temp = KKE_main_new();
  PointerLUXO ptr;
  LUXO_pointer_create(&LUXO_KrakenPixar, kmain_temp, &ptr);

  self->data_luxo = (KPy_StructLUXO *)pystage_struct_CreatePyObject(&ptr);

  PyObject_GC_Track(self);

  return (PyObject *)self->data_luxo;
}

static PyObject *kpy_pixar_data_context_exit(KPy_DataContext *self, PyObject *UNUSED(args))
{
  KKE_main_free((Main *)self->data_luxo->ptr.data);
  self->data_luxo->ptr.ptr = NULL;
  self->data_luxo->ptr.owner_id = NULL;
  Py_RETURN_NONE;
}

PyMethodDef KPY_pixar_data_context_method_def = {
  "temp_data",
  (PyCFunction)kpy_pixar_data_temp_data,
  METH_STATIC | METH_VARARGS | METH_KEYWORDS,
  kpy_pixar_data_context_load_doc,
};

int KPY_pixar_data_context_type_ready(void)
{
  if (PyType_Ready(&kpy_pixar_data_context_Type) < 0) {
    return -1;
  }

  return 0;
}

WABI_NAMESPACE_END