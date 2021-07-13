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

#include "UNI_object.h"
#include "UNI_wm_types.h"

#include "UNI_access.h"

#include "kpy_interface.h"
#include "kpy_intern_string.h"
#include "kpy_uni.h"

#define USE_PEDANTIC_WRITE
#define USE_MATHUTILS
#define USE_STRING_COERCE

#define USE_POSTPONED_ANNOTATIONS

WABI_NAMESPACE_BEGIN

KPy_UniverseObject *kpy_context_module = nullptr; /* for fast access */

static PyObject *pyuni_register_class(PyObject *self, PyObject *py_class);
static PyObject *pyuni_unregister_class(PyObject *self, PyObject *py_class);

static bool uni_disallow_writes = false;

static bool rna_id_write_error(PointerUNI *ptr, PyObject *key)
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

static int kpy_class_validate(UniverseObject *dummyptr, void *py_data, int *have_function)
{
  // return kpy_class_validate_recursive(dummyptr, dummyptr->type, py_data, have_function);
  return 1;
}

static int kpy_class_call(kContext *C, UniverseObject *ptr, void *func, UsdAttributeVector parms)
{
  return 1;
}

static void kpy_class_free(void *pyob_ptr)
{

}

UniverseObject *pyuni_struct_as_uni(PyObject *self, const bool parent, const char *error_prefix)
{
  KPy_UniverseObject *py_uni = NULL;
  UniverseObject *uni;

  /* Unfortunately PyObject_GetAttrString won't look up this types tp_dict first :/ */
  if (PyType_Check(self)) {
    py_uni = (KPy_UniverseObject *)PyDict_GetItem(((PyTypeObject *)self)->tp_dict, kpy_intern_str_kr_uni);
    Py_XINCREF(py_uni);
  }

  if (parent) {
    /* be very careful with this since it will return a parent classes uni.
     * modifying this will do confusing stuff! */
    if (py_uni == NULL) {
      py_uni = (KPy_UniverseObject *)PyObject_GetAttr(self, kpy_intern_str_kr_uni);
    }
  }

  if (py_uni == NULL) {
    PyErr_Format(PyExc_RuntimeError,
                 "%.200s, missing kr_uni attribute from '%.200s' instance (may not be registered)",
                 error_prefix,
                 Py_TYPE(self)->tp_name);
    return NULL;
  }

  if (!KPy_UniverseObject_Check(py_uni)) {
    PyErr_Format(PyExc_TypeError,
                 "%.200s, kr_uni attribute wrong type '%.200s' on '%.200s'' instance",
                 error_prefix,
                 Py_TYPE(py_uni)->tp_name,
                 Py_TYPE(self)->tp_name);
    Py_DECREF(py_uni);
    return NULL;
  }

  if (py_uni->ptr.type != &UNI_Object) {
    PyErr_Format(PyExc_TypeError,
                 "%.200s, kr_uni attribute not a UNI_Object, on '%.200s'' instance",
                 error_prefix,
                 Py_TYPE(self)->tp_name);
    Py_DECREF(py_uni);
    return NULL;
  }

  uni = (UniverseObject*)py_uni->ptr.data;
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
PyMethodDef meth_kpy_register_class = {
    "register_class", pyuni_register_class, METH_O, pyuni_register_class_doc};
static PyObject *pyuni_register_class(PyObject *UNUSED(self), PyObject *py_class)
{
  kContext *C = nullptr;
  ReportList reports;
  ObjectRegisterFunc reg;
  UniverseObject *uni;
  UniverseObject *uni_new;
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
  uni = pyuni_struct_as_uni(py_class, true, "register_class(...):");
  if (uni == NULL) {
    return NULL;
  }

  /* Check that we have a register callback for this type. */
  reg = UNI_object_register(uni);

  if (!reg) {
    PyErr_Format(PyExc_ValueError,
                 "register_class(...): expected a subclass of a registerable "
                 "UNI type (%.200s does not support registration)",
                 UNI_object_identifier(uni));
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
      }
      else {
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

WABI_NAMESPACE_END