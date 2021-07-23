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

#include "KPY_api.h"

#include "kpy.h"
#include "kpy_interface.h"

#include "WM_api.h"

WABI_NAMESPACE_BEGIN

static PyObject *kpy_atexit(PyObject *UNUSED(self), PyObject *UNUSED(args), PyObject *UNUSED(kw))
{
  /* close down enough of blender at least not to crash */
  struct kContext *C = KPY_context_get();

  //   WM_exit_ex(C, false);

  Py_RETURN_NONE;
}

static PyMethodDef meth_kpy_atexit = {"kpy_atexit", (PyCFunction)kpy_atexit, METH_NOARGS, NULL};
static PyObject *func_kpy_atregister = NULL; /* borrowed reference, `atexit` holds. */

static void atexit_func_call(const char *func_name, PyObject *atexit_func_arg)
{
  /* note - no error checking, if any of these fail we'll get a crash
   * this is intended, but if its problematic it could be changed */

  PyObject *atexit_mod = PyImport_ImportModuleLevel("atexit", NULL, NULL, NULL, 0);
  PyObject *atexit_func = PyObject_GetAttrString(atexit_mod, func_name);
  PyObject *args = PyTuple_New(1);
  PyObject *ret;

  PyTuple_SET_ITEM(args, 0, atexit_func_arg);
  Py_INCREF(atexit_func_arg); /* only incref so we don't dec'ref along with 'args' */

  ret = PyObject_CallObject(atexit_func, args);

  Py_DECREF(atexit_mod);
  Py_DECREF(atexit_func);
  Py_DECREF(args);

  if (ret)
  {
    Py_DECREF(ret);
  } else
  { /* should never happen */
    PyErr_Print();
  }
}


void KPY_atexit_register(void)
{
  /* atexit module owns this new function reference */
  KLI_assert(func_kpy_atregister == NULL);

  func_kpy_atregister = (PyObject *)PyCFunction_New(&meth_kpy_atexit, NULL);
  atexit_func_call("register", func_kpy_atregister);
}

void KPY_atexit_unregister(void)
{
  KLI_assert(func_kpy_atregister != NULL);

  atexit_func_call("unregister", func_kpy_atregister);
  func_kpy_atregister = NULL; /* don't really need to set but just in case */
}

WABI_NAMESPACE_END