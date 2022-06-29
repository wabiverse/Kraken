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
#include <stddef.h>

#include "KLI_path_utils.h"
#include "KLI_string_utils.h"
#include "KLI_utildefines.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_report.h"
#include "KKE_robinhood.h"
#include "KKE_utils.h"

#include "USD_file.h"
#include "USD_space_types.h"

#include "kpy_capi_utils.h"
#include "kpy_interface.h"
#include "kpy_library.h"
#include "kpy_stage.h"
#include "kpy_utildefines.h"

#include <wabi/usd/sdf/layer.h>
#include <wabi/usd/ar/resolver.h>
#include <wabi/usd/usd/primRange.h>
#include <wabi/usd/usd/stage.h>

WABI_NAMESPACE_BEGIN

#define INDEX_ID_MAX 41

struct KPy_Library
{
  PyObject_HEAD /* Required Python macro. */

    char relpath[FILE_MAX];
  char abspath[FILE_MAX];
  KrakenHandle *kr_handle;
  int flag;
  PyObject *dict;

  Main *kmain;
  bool kmain_is_temp;
};

static PyObject *kpy_lib_load(KPy_KrakenStage *self, PyObject *args, PyObject *kwds);
static PyObject *kpy_lib_enter(KPy_Library *self);
static PyObject *kpy_lib_exit(KPy_Library *self, PyObject *args);
static PyObject *kpy_lib_dir(KPy_Library *self);

static PyMethodDef kpy_lib_methods[] = {
  {"__enter__",   (PyCFunction)kpy_lib_enter, METH_NOARGS},
  {"__exit__",  (PyCFunction)kpy_lib_exit, METH_VARARGS},
  {"__dir__",    (PyCFunction)kpy_lib_dir, METH_NOARGS},
  {NULL}  /* sentinel */
};

static void kpy_lib_dealloc(KPy_Library *self)
{
  Py_XDECREF(self->dict);
  Py_TYPE(self)->tp_free(self);
}

static PyTypeObject kpy_lib_Type = {
  PyVarObject_HEAD_INIT(NULL, 0) "kpy_lib", /* tp_name */
  sizeof(KPy_Library),                      /* tp_basicsize */
  0,                                        /* tp_itemsize */
  /* methods */
  (destructor)kpy_lib_dealloc, /* tp_dealloc */
  0,                           /* tp_vectorcall_offset */
  NULL,                        /* getattrfunc tp_getattr; */
  NULL,                        /* setattrfunc tp_setattr; */
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
  0,
  /*** Added in release 2.2 ***/
  /*   Iterators */
  NULL, /* getiterfunc tp_iter; */
  NULL, /* iternextfunc tp_iternext; */

  /*** Attribute descriptor and subclassing stuff ***/
  kpy_lib_methods,             /* struct PyMethodDef *tp_methods; */
  NULL,                        /* struct PyMemberDef *tp_members; */
  NULL,                        /* struct PyGetSetDef *tp_getset; */
  NULL,                        /* struct _typeobject *tp_base; */
  NULL,                        /* PyObject *tp_dict; */
  NULL,                        /* descrgetfunc tp_descr_get; */
  NULL,                        /* descrsetfunc tp_descr_set; */
  offsetof(KPy_Library, dict), /* long tp_dictoffset; */
  NULL,                        /* initproc tp_init; */
  NULL,                        /* allocfunc tp_alloc; */
  NULL,                        /* newfunc tp_new; */
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

PyDoc_STRVAR(
  kpy_lib_load_doc,
  ".. method:: load(filepath, link=False, relative=False, assets_only=False)\n"
  "\n"
  "   Returns a context manager which exposes 2 library objects on entering.\n"
  "   Each object has attributes matching kpy.data which are lists of strings to be linked.\n"
  "\n"
  "   :arg filepath: The path to a blend file.\n"
  "   :type filepath: string\n"
  "   :arg link: When False reference to the original file is lost.\n"
  "   :type link: bool\n"
  "   :arg relative: When True the path is stored relative to the open blend file.\n"
  "   :type relative: bool\n"
  "   :arg assets_only: If True, only list data-blocks marked as assets.\n"
  "   :type assets_only: bool\n");
static PyObject *kpy_lib_load(KPy_KrakenStage *self, PyObject *args, PyObject *kw)
{
  Main *kmain_base = CTX_data_main(KPY_context_get());
  Main *kmain = (Main *)self->ptr.data; /* Typically #G_MAIN */
  KPy_Library *ret;
  const char *filename = NULL;
  bool is_rel = false, is_link = false, use_assets_only = false;

  static const char *_keywords[] = {"filepath", "link", "relative", "assets_only", NULL};
  static _PyArg_Parser _parser = {"s|$O&O&O&:load", _keywords, 0};
  if (!_PyArg_ParseTupleAndKeywordsFast(args,
                                        kw,
                                        &_parser,
                                        &filename,
                                        PyC_ParseBool,
                                        &is_link,
                                        PyC_ParseBool,
                                        &is_rel,
                                        PyC_ParseBool,
                                        &use_assets_only)) {
    return NULL;
  }

  ret = PyObject_New(KPy_Library, &kpy_lib_Type);

  KLI_strncpy(ret->relpath, filename, sizeof(ret->relpath));
  KLI_strncpy(ret->abspath, filename, sizeof(ret->abspath));
  //   KLI_path_abs(ret->abspath, KKE_main_pixarfile_path(kmain));

  ret->kmain = kmain;
  ret->kmain_is_temp = (kmain != kmain_base);

  ret->flag = ((is_link ? FILE_LINK : 0) | (is_rel ? FILE_RELPATH : 0) |
               (use_assets_only ? FILE_ASSETS_ONLY : 0));

  ret->dict = _PyDict_NewPresized(INDEX_ID_MAX);

  return (PyObject *)ret;
}

static const void _AppendToPaths(SdfPathVector paths, const SdfPath &path)
{
  paths.push_back(path);
}

static PyObject *_kpy_names(KPy_Library *self, const SdfPath &prim)
{
  PyObject *list;
  SdfPathVector paths;

  SdfLayerRefPtr layer = self->kr_handle->sdf_handle;

  SdfLayer::TraversalFunction appendFunc = std::bind(&_AppendToPaths,
                                                     paths,
                                                     std::placeholders::_1);
  layer->Traverse(prim, appendFunc);

  list = PyList_New(paths.size());

  if (!paths.empty()) {
    int counter = 0;
    UNIVERSE_FOR_ALL (sdf_path, paths) {
      PyList_SET_ITEM(list, counter, PyUnicode_FromString(CHARALL(sdf_path.GetName())));
      counter++;
    }
    paths.clear();
  }

  return list;
}

static PyObject *kpy_lib_enter(KPy_Library *self)
{
  PyObject *ret;
  KPy_Library *self_from;
  PyObject *from_dict = _PyDict_NewPresized(INDEX_ID_MAX);
  ReportList reports;

  KKE_reports_init(&reports, RPT_STORE);
  KrakenFileReadReport kr_reports = {};
  kr_reports.reports = &reports;

  self->kr_handle = KLO_krakenhandle_from_file(self->abspath, &kr_reports);

  if (self->kr_handle == NULL) {
    if (KPy_reports_to_error(&reports, PyExc_IOError, true) != -1) {
      PyErr_Format(PyExc_IOError, "load: %s failed to open kraken project file", self->abspath);
    }
    return NULL;
  }

  SdfLayer::RootPrimsView prims = self->kr_handle->sdf_handle->GetRootPrims();

  UNIVERSE_FOR_ALL (prim, prims) {
    PyObject *str = PyUnicode_FromString(CHARALL(prim->GetName()));
    PyObject *item;

    PyDict_SetItem(self->dict, str, item = PyList_New(0));
    Py_DECREF(item);
    PyDict_SetItem(from_dict, str, item = _kpy_names(self, prim->GetPath()));
    Py_DECREF(item);

    Py_DECREF(str);
  }

  /* create a dummy */
  self_from = PyObject_New(KPy_Library, &kpy_lib_Type);
  KLI_strncpy(self_from->relpath, self->relpath, sizeof(self_from->relpath));
  KLI_strncpy(self_from->abspath, self->abspath, sizeof(self_from->abspath));

  self_from->kr_handle = nullptr;
  self_from->flag = 0;
  self_from->dict = from_dict; /* owns the dict */

  /* return pair */
  ret = PyTuple_New(2);
  PyTuple_SET_ITEMS(ret, (PyObject *)self_from, (PyObject *)self);
  Py_INCREF(self);

  KKE_reports_clear(&reports);

  return ret;
}

static PyObject *kpy_lib_exit(KPy_Library *self, PyObject *UNUSED(args))
{
  Py_RETURN_NONE;
}

static PyObject *kpy_lib_dir(KPy_Library *self)
{
  return PyDict_Keys(self->dict);
}

PyMethodDef KPY_library_load_method_def = {
  "load",
  (PyCFunction)kpy_lib_load,
  METH_VARARGS | METH_KEYWORDS,
  kpy_lib_load_doc,
};

int KPY_library_load_type_ready(void)
{
  if (PyType_Ready(&kpy_lib_Type) < 0) {
    return -1;
  }
  return 0;
}

WABI_NAMESPACE_END