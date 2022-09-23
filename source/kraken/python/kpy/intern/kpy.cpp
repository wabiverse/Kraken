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

#include "kraken/kraken.h"

#include "KLI_utildefines.h"
#include "KLI_string.h"

#include "KKE_appdir.h"
#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_robinhood.h"
#include "KKE_utils.h"

#include "LUXO_access.h"
#include "LUXO_runtime.h"

#include "USD_factory.h"
#include "USD_types.h"
#include "USD_wm_types.h"

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

#include <boost/python.hpp>
#include <boost/python/overloads.hpp>

using namespace boost::python;

KRAKEN_NAMESPACE_USING

PyObject *kpy_package_py = nullptr;

PyDoc_STRVAR(kpy_script_paths_doc,
             ".. function:: script_paths()\n"
             "\n"
             "   Return 2 paths to kraken scripts directories.\n"
             "\n"
             "   :return: (system, user) strings will be empty when not found.\n"
             "   :rtype: tuple of strings\n");
static PyObject *kpy_script_paths(PyObject *UNUSED(self))
{
  PyObject *ret = PyTuple_New(2);
  PyObject *item;
  const char *path;

  path = KKE_appdir_folder_id(KRAKEN_SYSTEM_SCRIPTS, NULL);
  item = PyC_UnicodeFromByte(path ? path : "");
  KLI_assert(item != NULL);
  PyTuple_SET_ITEM(ret, 0, item);
  path = KKE_appdir_folder_id(KRAKEN_USER_SCRIPTS, NULL);
  item = PyC_UnicodeFromByte(path ? path : "");
  KLI_assert(item != NULL);
  PyTuple_SET_ITEM(ret, 1, item);

  return ret;
}

PyDoc_STRVAR(kpy_resolver_paths_doc,
             ".. function:: resolver_paths(absolute=False, packed=False, local=False)\n"
             "\n"
             "   Returns a list of paths to external assets referenced by the loaded .usd file.\n"
             "\n"
             "   :arg absolute: When true the paths returned are made absolute.\n"
             "   :type absolute: boolean\n"
             "   :arg packed: When true skip file paths for packed data.\n"
             "   :type packed: boolean\n"
             "   :arg local: When true skip linked library paths.\n"
             "   :type local: boolean\n"
             "   :return: path list.\n"
             "   :rtype: list of strings\n");
static PyObject *kpy_resolver_paths(PyObject *UNUSED(self), PyObject *args, PyObject *kw)
{
  int flag = 0;
  PyObject *list;

  bool absolute = false;
  bool packed = false;
  bool local = false;

  static const char *_keywords[] = {"absolute", "packed", "local", NULL};
  static _PyArg_Parser _parser = {
    "|$" /* Optional keyword only arguments. */
    "O&" /* `absolute` */
    "O&" /* `packed` */
    "O&" /* `local` */
    ":resolver_paths",
    _keywords,
    0,
  };
  if (!_PyArg_ParseTupleAndKeywordsFast(args,
                                        kw,
                                        &_parser,
                                        PyC_ParseBool,
                                        &absolute,
                                        PyC_ParseBool,
                                        &packed,
                                        PyC_ParseBool,
                                        &local)) {
    return NULL;
  }

  if (absolute) {
    flag |= KKE_SDFPATH_FOREACH_PATH_ABSOLUTE;
  }
  if (!packed) {
    flag |= KKE_SDFPATH_FOREACH_PATH_SKIP_PACKED;
  }
  if (local) {
    flag |= KKE_SDFPATH_FOREACH_PATH_SKIP_LINKED;
  }

  list = PyList_New(0);

  return list;
}

static PyObject *kpy_user_resource(PyObject *UNUSED(self), PyObject *args, PyObject *kw)
{
  const struct PyC_StringEnumItems type_items[] = {
    {KRAKEN_USER_DATAFILES, "DATAFILES"},
    {KRAKEN_USER_CONFIG,    "CONFIG"   },
    {KRAKEN_USER_SCRIPTS,   "SCRIPTS"  },
    {KRAKEN_USER_AUTOSAVE,  "AUTOSAVE" },
    {0,                     NULL       },
  };
  struct PyC_StringEnum type = {type_items};

  const char *subdir = NULL;

  const char *path;

  static const char *_keywords[] = {"type", "path", NULL};
  static _PyArg_Parser _parser = {
    "O&" /* `type` */
    "|$" /* Optional keyword only arguments. */
    "s"  /* `path` */
    ":user_resource",
    _keywords,
    0,
  };
  if (!_PyArg_ParseTupleAndKeywordsFast(args, kw, &_parser, PyC_ParseStringEnum, &type, &subdir)) {
    return NULL;
  }

  /* same logic as KKE_appdir_folder_id_create(),
   * but best leave it up to the script author to create */
  path = KKE_appdir_folder_id_user_notest(type.value_found, subdir);

  return PyC_UnicodeFromByte(path ? path : "");
}

PyDoc_STRVAR(kpy_system_resource_doc,
             ".. function:: system_resource(type, path=\"\")\n"
             "\n"
             "   Return a system resource path.\n"
             "\n"
             "   :arg type: string in ['DATAFILES', 'SCRIPTS', 'PYTHON'].\n"
             "   :type type: string\n"
             "   :arg path: Optional subdirectory.\n"
             "   :type path: string\n");
static PyObject *kpy_system_resource(PyObject *UNUSED(self), PyObject *args, PyObject *kw)
{
  const struct PyC_StringEnumItems type_items[] = {
    {KRAKEN_SYSTEM_DATAFILES, "DATAFILES"},
    {KRAKEN_SYSTEM_SCRIPTS,   "SCRIPTS"  },
    {KRAKEN_SYSTEM_PYTHON,    "PYTHON"   },
    {0,                       NULL       },
  };
  struct PyC_StringEnum type = {type_items};

  const char *subdir = NULL;

  const char *path;

  static const char *_keywords[] = {"type", "path", NULL};
  static _PyArg_Parser _parser = {"O&|$s:system_resource", _keywords, 0};
  if (!_PyArg_ParseTupleAndKeywordsFast(args, kw, &_parser, PyC_ParseStringEnum, &type, &subdir)) {
    return NULL;
  }

  path = KKE_appdir_folder_id(type.value_found, subdir);

  return PyC_UnicodeFromByte(path ? path : "");
}

PyDoc_STRVAR(
  kpy_resource_path_doc,
  ".. function:: resource_path(type, major=kpy.app.version[0], minor=kpy.app.version[1])\n"
  "\n"
  "   Return the base path for storing system files.\n"
  "\n"
  "   :arg type: string in ['USER', 'LOCAL', 'SYSTEM'].\n"
  "   :type type: string\n"
  "   :arg major: major version, defaults to current.\n"
  "   :type major: int\n"
  "   :arg minor: minor version, defaults to current.\n"
  "   :type minor: string\n"
  "   :return: the resource path (not necessarily existing).\n"
  "   :rtype: string\n");
static PyObject *kpy_resource_path(PyObject *UNUSED(self), PyObject *args, PyObject *kw)
{
  const struct PyC_StringEnumItems type_items[] = {
    {KRAKEN_RESOURCE_PATH_USER,   "USER"  },
    {KRAKEN_RESOURCE_PATH_LOCAL,  "LOCAL" },
    {KRAKEN_RESOURCE_PATH_SYSTEM, "SYSTEM"},
    {0,                           NULL    },
  };
  struct PyC_StringEnum type = {type_items};

  int major = KRAKEN_VERSION / 100, minor = KRAKEN_VERSION % 100;
  const char *path;

  static const char *_keywords[] = {"type", "major", "minor", NULL};
  static _PyArg_Parser _parser = {
    "O&" /* `type` */
    "|$" /* Optional keyword only arguments. */
    "i"  /* `major` */
    "i"  /* `minor` */
    ":resource_path",
    _keywords,
    0,
  };
  if (!_PyArg_ParseTupleAndKeywordsFast(args,
                                        kw,
                                        &_parser,
                                        PyC_ParseStringEnum,
                                        &type,
                                        &major,
                                        &minor)) {
    return NULL;
  }

  path = KKE_appdir_folder_id_version(type.value_found, (major * 100) + minor, false);

  return PyC_UnicodeFromByte(path ? path : "");
}

PyDoc_STRVAR(kpy_escape_identifier_doc,
             ".. function:: escape_identifier(string)\n"
             "\n"
             "   Simple string escaping function used for animation paths.\n"
             "\n"
             "   :arg string: text\n"
             "   :type string: string\n"
             "   :return: The escaped string.\n"
             "   :rtype: string\n");
static PyObject *kpy_escape_identifier(PyObject *UNUSED(self), PyObject *value)
{
  Py_ssize_t value_str_len;
  const char *value_str = PyUnicode_AsUTF8AndSize(value, &value_str_len);

  if (value_str == NULL) {
    PyErr_SetString(PyExc_TypeError, "expected a string");
    return NULL;
  }

  const size_t size = (value_str_len * 2) + 1;
  char *value_escape_str = (char *)PyMem_MALLOC(size);
  const Py_ssize_t value_escape_str_len = KLI_str_escape(value_escape_str, value_str, size);

  PyObject *value_escape;
  if (value_escape_str_len == value_str_len) {
    Py_INCREF(value);
    value_escape = value;
  } else {
    value_escape = PyUnicode_FromStringAndSize(value_escape_str, value_escape_str_len);
  }

  PyMem_FREE(value_escape_str);

  return value_escape;
}

PyDoc_STRVAR(kpy_unescape_identifier_doc,
             ".. function:: unescape_identifier(string)\n"
             "\n"
             "   Simple string un-escape function used for animation paths.\n"
             "   This performs the reverse of `escape_identifier`.\n"
             "\n"
             "   :arg string: text\n"
             "   :type string: string\n"
             "   :return: The un-escaped string.\n"
             "   :rtype: string\n");
static PyObject *kpy_unescape_identifier(PyObject *UNUSED(self), PyObject *value)
{
  Py_ssize_t value_str_len;
  const char *value_str = PyUnicode_AsUTF8AndSize(value, &value_str_len);

  if (value_str == NULL) {
    PyErr_SetString(PyExc_TypeError, "expected a string");
    return NULL;
  }

  const size_t size = value_str_len + 1;
  char *value_unescape_str = (char *)PyMem_MALLOC(size);
  const Py_ssize_t value_unescape_str_len = KLI_str_unescape(value_unescape_str, value_str, size);

  PyObject *value_unescape;
  if (value_unescape_str_len == value_str_len) {
    Py_INCREF(value);
    value_unescape = value;
  } else {
    value_unescape = PyUnicode_FromStringAndSize(value_unescape_str, value_unescape_str_len);
  }

  PyMem_FREE(value_unescape_str);

  return value_unescape;
}

static PyMethodDef meth_kpy_script_paths = {
  "script_paths",
  (PyCFunction)kpy_script_paths,
  METH_NOARGS,
  kpy_script_paths_doc,
};
static PyMethodDef meth_kpy_resolver_paths = {
  "resolver_paths",
  (PyCFunction)kpy_resolver_paths,
  METH_VARARGS | METH_KEYWORDS,
  kpy_resolver_paths_doc,
};
static PyMethodDef meth_kpy_user_resource = {
  "user_resource",
  (PyCFunction)kpy_user_resource,
  METH_VARARGS | METH_KEYWORDS,
  NULL,
};
static PyMethodDef meth_kpy_system_resource = {
  "system_resource",
  (PyCFunction)kpy_system_resource,
  METH_VARARGS | METH_KEYWORDS,
  kpy_system_resource_doc,
};
static PyMethodDef meth_kpy_resource_path = {
  "resource_path",
  (PyCFunction)kpy_resource_path,
  METH_VARARGS | METH_KEYWORDS,
  kpy_resource_path_doc,
};
static PyMethodDef meth_kpy_escape_identifier = {
  "escape_identifier",
  (PyCFunction)kpy_escape_identifier,
  METH_O,
  kpy_escape_identifier_doc,
};
static PyMethodDef meth_kpy_unescape_identifier = {
  "unescape_identifier",
  (PyCFunction)kpy_unescape_identifier,
  METH_O,
  kpy_unescape_identifier_doc,
};

static PyObject *kpy_import_test(const char *modname)
{
  PyObject *mod = PyImport_ImportModuleLevel(modname, NULL, NULL, NULL, 0);

  // GPU_kvk_end();

  if (mod) {
    Py_DECREF(mod);
  } else {
    PyErr_Print();
    PyErr_Clear();
  }

  return mod;
}

/******************************************************************************
 * Description: Creates the kpy module and adds it to sys.modules for importing
 ******************************************************************************/
void KPy_init_modules(struct kraken::kContext *C)
{
  KrakenPRIM ctx_ptr;
  PyObject *mod;

  /* Needs to be first since this dir is needed for future modules */
  const char *const modpath = KKE_appdir_folder_id(KRAKEN_SYSTEM_SCRIPTS, "modules");

  if (modpath) {
    // printf("kpy: found module path '%s'.\n", modpath);
    PyObject *sys_path = PySys_GetObject("path"); /* borrow */
    PyObject *py_modpath = PyUnicode_FromString(modpath);
    PyList_Insert(sys_path, 0, py_modpath); /* add first */
    Py_DECREF(py_modpath);
  } else {
    printf("kpy: couldn't find 'scripts/modules', kraken probably won't start.\n");
  }
  /* stand alone utility modules not related to kraken directly */
  // IDProp_Init_Types(); /* not actually a submodule, just types */

  mod = PyModule_New("_kpy");

  /* add the module so we can import it */
  PyDict_SetItemString(PyImport_GetModuleDict(), "_kpy", mod);
  Py_DECREF(mod);

  /* needs to be first so kpy_types can run */
  PyModule_AddObject(mod, "types", KPY_uni_types());

  /* The entirety of Pixar USD python bindings... */
  PyModule_AddObject(mod, "Tf", PyInit__tf());
  PyModule_AddObject(mod, "Gf", PyInit__gf());
  PyModule_AddObject(mod, "Trace", PyInit__trace());
  PyModule_AddObject(mod, "Work", PyInit__work());
  PyModule_AddObject(mod, "Plug", PyInit__plug());
  PyModule_AddObject(mod, "Vt", PyInit__vt());
  PyModule_AddObject(mod, "Ar", PyInit__ar());
  PyModule_AddObject(mod, "Kind", PyInit__kind());
  PyModule_AddObject(mod, "Sdf", PyInit__sdf());
  PyModule_AddObject(mod, "Ndr", PyInit__ndr());
  PyModule_AddObject(mod, "Sdr", PyInit__sdr());
  PyModule_AddObject(mod, "Pcp", PyInit__pcp());
  PyModule_AddObject(mod, "Usd", PyInit__usd());
  PyModule_AddObject(mod, "UsdGeom", PyInit__usdGeom());
  PyModule_AddObject(mod, "UsdVol", PyInit__usdVol());
  PyModule_AddObject(mod, "UsdMedia", PyInit__usdMedia());
  PyModule_AddObject(mod, "UsdShade", PyInit__usdShade());
  PyModule_AddObject(mod, "UsdLux", PyInit__usdLux());
  PyModule_AddObject(mod, "UsdRender", PyInit__usdRender());
  PyModule_AddObject(mod, "UsdHydra", PyInit__usdHydra());
  PyModule_AddObject(mod, "UsdRi", PyInit__usdRi());
  PyModule_AddObject(mod, "UsdSkel", PyInit__usdSkel());
  PyModule_AddObject(mod, "UsdUI", PyInit__usdUI());
  PyModule_AddObject(mod, "UsdUtils", PyInit__usdUtils());
  PyModule_AddObject(mod, "UsdPhysics", PyInit__usdPhysics());
  PyModule_AddObject(mod, "UsdAbc", PyInit__usdAbc());
  PyModule_AddObject(mod, "UsdDraco", PyInit__usdDraco());
  PyModule_AddObject(mod, "Garch", PyInit__garch());
  PyModule_AddObject(mod, "CameraUtil", PyInit__cameraUtil());
  PyModule_AddObject(mod, "PxOsd", PyInit__pxOsd());
  PyModule_AddObject(mod, "Glf", PyInit__glf());
  PyModule_AddObject(mod, "UsdImagingGL", PyInit__usdImagingGL());
  PyModule_AddObject(mod, "UsdAppUtils", PyInit__usdAppUtils());
  PyModule_AddObject(mod, "Usdviewq", PyInit__usdviewq());

  /* needs to be first so kpy_types can run */
  KPY_library_load_type_ready();

  KPY_pixar_data_context_type_ready();

  kpy_import_test("kpy_types");
  PyModule_AddObject(mod, "data", KPY_stage_module()); /* imports kpy_types by running this */
  kpy_import_test("kpy_types");

  PyModule_AddObject(mod, "app", KPY_app_struct());

  LUXO_pointer_create(&LUXO_Context, C, &ctx_ptr);
  kpy_context_module = (KPy_KrakenStage *)pystage_struct_CreatePyObject(&ctx_ptr);
  /* odd that this is needed, 1 ref on creation and another for the module
   * but without we get a crash on exit */
  Py_INCREF(kpy_context_module);
  PyModule_AddObject(mod, "context", (PyObject *)kpy_context_module);

  /* Register methods and property get/set for RNA types. */
  //   KPY_uni_types_extend_capi();

  /* utility func's that have nowhere else to go */
  PyModule_AddObject(mod,
                     meth_kpy_script_paths.ml_name,
                     (PyObject *)PyCFunction_New(&meth_kpy_script_paths, NULL));
  PyModule_AddObject(mod,
                     meth_kpy_resolver_paths.ml_name,
                     (PyObject *)PyCFunction_New(&meth_kpy_resolver_paths, NULL));
  PyModule_AddObject(mod,
                     meth_kpy_user_resource.ml_name,
                     (PyObject *)PyCFunction_New(&meth_kpy_user_resource, NULL));
  PyModule_AddObject(mod,
                     meth_kpy_system_resource.ml_name,
                     (PyObject *)PyCFunction_New(&meth_kpy_system_resource, NULL));
  PyModule_AddObject(mod,
                     meth_kpy_resource_path.ml_name,
                     (PyObject *)PyCFunction_New(&meth_kpy_resource_path, NULL));
  PyModule_AddObject(mod,
                     meth_kpy_escape_identifier.ml_name,
                     (PyObject *)PyCFunction_New(&meth_kpy_escape_identifier, NULL));
  PyModule_AddObject(mod,
                     meth_kpy_unescape_identifier.ml_name,
                     (PyObject *)PyCFunction_New(&meth_kpy_unescape_identifier, NULL));

  /* register funcs (kpy_stage.c) */
  PyModule_AddObject(mod,
                     meth_kpy_register_class.ml_name,
                     (PyObject *)PyCFunction_New(&meth_kpy_register_class, NULL));
  PyModule_AddObject(mod,
                     meth_kpy_unregister_class.ml_name,
                     (PyObject *)PyCFunction_New(&meth_kpy_unregister_class, NULL));

  PyModule_AddObject(mod,
                     meth_kpy_owner_id_get.ml_name,
                     (PyObject *)PyCFunction_New(&meth_kpy_owner_id_get, NULL));
  PyModule_AddObject(mod,
                     meth_kpy_owner_id_set.ml_name,
                     (PyObject *)PyCFunction_New(&meth_kpy_owner_id_set, NULL));

  /* add our own modules dir, this is a python package */
  kpy_package_py = kpy_import_test("kpy");
}
