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

#include "kpy_app.h"

#include "kpy_capi_packarray.h"

#include "kpy.h"
#include "kpy_app.h"
#include "kpy_interface.h"
#include "kpy_intern_string.h"
#include "kpy_path.h"
#include "kpy_stage.h"

#include "KKE_appdir.h"
#include "KKE_main.h"

#include "WM_event_system.h"

#include "kpy_capi_utils.h"
#include "kpy_driver.h"
#include "kpy_utildefines.h"

#include <wabi/base/tf/iterator.h>

#include <boost/python.hpp>
#include <boost/python/overloads.hpp>

using namespace boost::python;

KRAKEN_NAMESPACE_USING

#ifdef BUILD_DATE
extern char build_date[];
extern char build_time[];
extern ulong build_commit_timestamp;
extern char build_commit_date[];
extern char build_commit_time[];
extern char build_hash[];
extern char build_branch[];
extern char build_platform[];
extern char build_type[];
extern char build_cflags[];
extern char build_cxxflags[];
extern char build_linkflags[];
extern char build_system[];
#endif

static PyTypeObject KrakenAppType;

static PyStructSequence_Field app_info_fields[] = {
  {"version",                                                                         "The Kraken version as a tuple of 3 numbers. eg. (2, 83, 1)"},
  {"version_file",
   "The Kraken version, as a tuple, last used to save a .blend file, compatible with "
   "``kpy.data.version``. This value should be used for handling compatibility changes between "
   "Kraken versions"},
  {"version_string",                                                                 "The Kraken version formatted as a string"},
  {"version_cycle",                                                                  "The release status of this build alpha/beta/rc/release"},
  {"binary_path",
   "The location of Kraken's executable, useful for utilities that open new instances"},
  {"background",
   "Boolean, True when kraken is running without a user interface (started with -b)"},
  {"factory_startup",                                                                     "Boolean, True when kraken is running with --factory-startup)"},

 /* buildinfo */
  {"build_date",                                                                                 "The date this kraken instance was built"},
  {"build_time", "The time this kraken instance was built"},
  {"build_commit_timestamp",                                                                 "The unix timestamp of commit this kraken instance was built"},
  {"build_commit_date",                                                                                 "The date of commit this kraken instance was built"},
  {"build_commit_time","The time of commit this kraken instance was built"},
  {"build_hash",                                                                      "The commit hash this kraken instance was built with"},
  {"build_branch",                                                                                 "The branch this kraken instance was built from"},
  {"build_platform","The platform this kraken instance was built for"},
  {"build_type",                                                               "The type of build (Release, Debug)"},
  {"build_cflags",                                                                                 "C compiler flags"},
  {"build_cxxflags",                                                                       "C++ compiler flags"},
  {"build_linkflags",                                                                      "Binary linking flags"},
  {"build_system",                                                                                 "Build system used"},
  {NULL},
};

PyDoc_STRVAR(kpy_app_doc,
             "This module contains application values that remain unchanged during runtime.");

static PyStructSequence_Desc app_info_desc = {
  "kpy.app",       /* name */
  kpy_app_doc,     /* doc */
  app_info_fields, /* fields */
  ARRAY_SIZE(app_info_fields) - 1,
};

static PyObject *make_app_info(void)
{
  PyObject *app_info;
  int pos = 0;

  app_info = PyStructSequence_New(&KrakenAppType);
  if (app_info == NULL) {
    return NULL;
  }
#define SetIntItem(flag) PyStructSequence_SET_ITEM(app_info, pos++, PyLong_FromLong(flag))
#define SetStrItem(str) PyStructSequence_SET_ITEM(app_info, pos++, PyUnicode_FromString(str))
#define SetBytesItem(str) PyStructSequence_SET_ITEM(app_info, pos++, PyBytes_FromString(str))
#define SetObjItem(obj) PyStructSequence_SET_ITEM(app_info, pos++, obj)

  SetObjItem(PyC_Tuple_Pack_I32(KRAKEN_VERSION / 100, KRAKEN_VERSION % 100, KRAKEN_VERSION_PATCH));
  SetObjItem(PyC_Tuple_Pack_I32(KRAKEN_FILE_VERSION / 100,
                                KRAKEN_FILE_VERSION % 100,
                                KRAKEN_FILE_SUBVERSION));
  SetStrItem(KKE_kraken_version_string());
  SetStrItem(STRINGIFY(KRAKEN_VERSION_CYCLE));
  SetStrItem(KKE_appdir_program_path());
  SetObjItem(PyBool_FromLong(G.background));
  SetObjItem(PyBool_FromLong(G.factory_startup));

#ifdef BUILD_DATE
  SetBytesItem(build_date);
  SetBytesItem(build_time);
  SetIntItem(build_commit_timestamp);
  SetBytesItem(build_commit_date);
  SetBytesItem(build_commit_time);
  SetBytesItem(build_hash);
  SetBytesItem(build_branch);
  SetBytesItem(build_platform);
  SetBytesItem(build_type);
  SetBytesItem(build_cflags);
  SetBytesItem(build_cxxflags);
  SetBytesItem(build_linkflags);
  SetBytesItem(build_system);
#else
  SetBytesItem("Unknown");
  SetBytesItem("Unknown");
  SetIntItem(0);
  SetBytesItem("Unknown");
  SetBytesItem("Unknown");
  SetBytesItem("Unknown");
  SetBytesItem("Unknown");
  SetBytesItem("Unknown");
  SetBytesItem("Unknown");
  SetBytesItem("Unknown");
  SetBytesItem("Unknown");
  SetBytesItem("Unknown");
  SetBytesItem("Unknown");
#endif

#undef SetIntItem
#undef SetStrItem
#undef SetBytesItem
#undef SetObjItem

  if (PyErr_Occurred()) {
    Py_DECREF(app_info);
    return NULL;
  }
  return app_info;
}

/* a few getsets because it makes sense for them to be in kpy.app even though
 * they are not static */

PyDoc_STRVAR(
  kpy_app_debug_doc,
  "Boolean, for debug info (started with --debug / --debug_* matching this attribute name)");
static PyObject *kpy_app_debug_get(PyObject *UNUSED(self), void *closure)
{
  const int flag = POINTER_AS_INT(closure);
  return PyBool_FromLong(G.debug & flag);
}

static int kpy_app_debug_set(PyObject *UNUSED(self), PyObject *value, void *closure)
{
  const int flag = POINTER_AS_INT(closure);
  const int param = PyObject_IsTrue(value);

  if (param == -1) {
    PyErr_SetString(PyExc_TypeError, "kpy.app.debug can only be True/False");
    return -1;
  }

  if (param) {
    G.debug |= flag;
  } else {
    G.debug &= ~flag;
  }

  return 0;
}

PyDoc_STRVAR(
  kpy_app_global_flag_doc,
  "Boolean, for application behavior (started with --enable-* matching this attribute name)");
static PyObject *kpy_app_global_flag_get(PyObject *UNUSED(self), void *closure)
{
  const int flag = POINTER_AS_INT(closure);
  return PyBool_FromLong(G.f & flag);
}

static int kpy_app_global_flag_set(PyObject *UNUSED(self), PyObject *value, void *closure)
{
  const int flag = POINTER_AS_INT(closure);
  const int param = PyObject_IsTrue(value);

  if (param == -1) {
    PyErr_SetString(PyExc_TypeError, "kpy.app.use_* can only be True/False");
    return -1;
  }

  if (param) {
    G.f |= flag;
  } else {
    G.f &= ~flag;
  }

  return 0;
}

static int kpy_app_global_flag_set__only_disable(PyObject *UNUSED(self),
                                                 PyObject *value,
                                                 void *closure)
{
  const int param = PyObject_IsTrue(value);
  if (param == 1) {
    PyErr_SetString(PyExc_ValueError, "This kpy.app.use_* option can only be disabled");
    return -1;
  }
  return kpy_app_global_flag_set(NULL, value, closure);
}

PyDoc_STRVAR(kpy_app_debug_value_doc,
             "Short, number which can be set to non-zero values for testing purposes");
static PyObject *kpy_app_debug_value_get(PyObject *UNUSED(self), void *UNUSED(closure))
{
  return PyLong_FromLong(G.debug_value);
}

static int kpy_app_debug_value_set(PyObject *UNUSED(self), PyObject *value, void *UNUSED(closure))
{
  const short param = PyC_Long_AsI16(value);

  if (param == -1 && PyErr_Occurred()) {
    PyC_Err_SetString_Prefix(PyExc_TypeError,
                             "kpy.app.debug_value can only be set to a whole number");
    return -1;
  }

  G.debug_value = param;

  WM_main_add_notifier(NC_WINDOW, NULL);

  return 0;
}

PyDoc_STRVAR(kpy_app_tempdir_doc, "String, the temp directory used by kraken (read-only)");
static PyObject *kpy_app_tempdir_get(PyObject *UNUSED(self), void *UNUSED(closure))
{
  return PyC_UnicodeFromByte(KKE_tempdir_session());
}

static PyObject *kpy_app_autoexec_fail_message_get(PyObject *UNUSED(self), void *UNUSED(closure))
{
  return PyC_UnicodeFromByte(G.autoexec_fail);
}

static PyGetSetDef kpy_app_getsets[] = {
  {"debug",                 kpy_app_debug_get,                 kpy_app_debug_set,       kpy_app_debug_doc,       (void *)G_DEBUG                          },
  {"debug_python",
   kpy_app_debug_get,                                          kpy_app_debug_set,
   kpy_app_debug_doc,                                                                                            (void *)G_DEBUG_PYTHON                   },
  {"debug_wm",              kpy_app_debug_get,                 kpy_app_debug_set,       kpy_app_debug_doc,       (void *)G_DEBUG_WM                       },
  {"debug_io",              kpy_app_debug_get,                 kpy_app_debug_set,       kpy_app_debug_doc,       (void *)G_DEBUG_IO                       },
  {"debug_value",           kpy_app_debug_value_get,           kpy_app_debug_value_set, kpy_app_debug_value_doc, NULL                                     },
  {"tempdir",               kpy_app_tempdir_get,               NULL,                    kpy_app_tempdir_doc,     NULL                                     },
  {"autoexec_fail",         kpy_app_global_flag_get,           NULL,                    NULL,                    (void *)G_FLAG_SCRIPT_AUTOEXEC_FAIL      },
  {"autoexec_fail_quiet",
   kpy_app_global_flag_get,                                    NULL,
   NULL,                                                                                                         (void *)G_FLAG_SCRIPT_AUTOEXEC_FAIL_QUIET},
  {"autoexec_fail_message", kpy_app_autoexec_fail_message_get, NULL,                    NULL,                    NULL                                     },
  {NULL,                    NULL,                              NULL,                    NULL,                    NULL                                     },
};

static void py_struct_seq_getset_init(void)
{
  /* tricky dynamic members, not to py-spec! */
  for (PyGetSetDef *getset = kpy_app_getsets; getset->name; getset++) {
    PyObject *item = PyDescr_NewGetSet(&KrakenAppType, getset);
    PyDict_SetItem(KrakenAppType.tp_dict, PyDescr_NAME(item), item);
    Py_DECREF(item);
  }
}
/* end dynamic kpy.app */

PyObject *KPY_app_struct(void)
{
  PyObject *ret;

  PyStructSequence_InitType(&KrakenAppType, &app_info_desc);

  ret = make_app_info();

  /* prevent user from creating new instances */
  KrakenAppType.tp_init = NULL;
  KrakenAppType.tp_new = NULL;

  /* without this we can't do set(sys.modules). */
  KrakenAppType.tp_hash = (hashfunc)_Py_HashPointer;

  /* kindof a hack ontop of PyStructSequence */
  py_struct_seq_getset_init();

  return ret;
}
