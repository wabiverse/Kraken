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

#include "KLI_math_inline.h"
#include "KLI_string_utils.h"

#include "KKE_utils.h"
#include "KKE_main.h"

#include "LUXO_runtime.h"

#include "USD_types.h"

#include "kpy_driver.h"
#include "kpy_intern_string.h"
#include "kpy_stage.h"

#include "KPY_extern_python.h"

#define USE_USD_AS_PYOBJECT

#define USE_BYTECODE_WHITELIST

#ifdef USE_BYTECODE_WHITELIST
#  include <opcode.h>
#endif

/**
 * For PyDrivers
 * (drivers using one-line Python expressions to express relationships between targets). */
PyObject *kpy_pydriver_Dict = NULL;

#ifdef USE_BYTECODE_WHITELIST
static PyObject *kpy_pydriver_Dict__whitelist = NULL;
#endif

/* For faster execution we keep a special dictionary for pydrivers, with
 * the needed modules and aliases.
 */
int kpy_pydriver_create_dict(void)
{
  PyObject *d, *mod;

  /* validate namespace for driver evaluation */
  if (kpy_pydriver_Dict) {
    return -1;
  }

  d = PyDict_New();
  if (d == NULL) {
    return -1;
  }

  kpy_pydriver_Dict = d;

  /* Import some modules: builtins, kpy, math. */
  PyDict_SetItemString(d, "__builtins__", PyEval_GetBuiltins());

  mod = PyImport_ImportModule("math");
  if (mod) {
    PyDict_Merge(d, PyModule_GetDict(mod), 0); /* 0 - don't overwrite existing values */
    Py_DECREF(mod);
  }
#ifdef USE_BYTECODE_WHITELIST
  PyObject *mod_math = mod;
#endif

  /* add kpy to global namespace */
  mod = PyImport_ImportModuleLevel("kpy", NULL, NULL, NULL, 0);
  if (mod) {
    PyDict_SetItemString(kpy_pydriver_Dict, "kpy", mod);
    Py_DECREF(mod);
  }

  /* Add math utility functions. */
  mod = PyImport_ImportModuleLevel("kr_math", NULL, NULL, NULL, 0);
  if (mod) {
    static const char *names[] = {"clamp", "lerp", "smoothstep", NULL};

    for (const char **pname = names; *pname; ++pname) {
      PyObject *func = PyDict_GetItemString(PyModule_GetDict(mod), *pname);
      PyDict_SetItemString(kpy_pydriver_Dict, *pname, func);
    }

    Py_DECREF(mod);
  }

#ifdef USE_BYTECODE_WHITELIST
  /* setup the whitelist */
  {
    kpy_pydriver_Dict__whitelist = PyDict_New();
    const char *whitelist[] = {
      /* builtins (basic) */
      "all",
      "any",
      "len",
      /* builtins (numeric) */
      "max",
      "min",
      "pow",
      "round",
      "sum",
      /* types */
      "bool",
      "float",
      "int",
      /* kr_math */
      "clamp",
      "lerp",
      "smoothstep",

      NULL,
    };

    for (int i = 0; whitelist[i]; i++) {
      PyDict_SetItemString(kpy_pydriver_Dict__whitelist, whitelist[i], Py_None);
    }

    /* Add all of 'math' functions. */
    if (mod_math != NULL) {
      PyObject *mod_math_dict = PyModule_GetDict(mod_math);
      PyObject *arg_key, *arg_value;
      Py_ssize_t arg_pos = 0;
      while (PyDict_Next(mod_math_dict, &arg_pos, &arg_key, &arg_value)) {
        const char *arg_str = PyUnicode_AsUTF8(arg_key);
        if (arg_str[0] && arg_str[1] != '_') {
          PyDict_SetItem(kpy_pydriver_Dict__whitelist, arg_key, Py_None);
        }
      }
    }
  }
#endif /* USE_BYTECODE_WHITELIST */

  return 0;
}

/* note, this function should do nothing most runs, only when changing frame */
/* not thread safe but neither is python */
struct DriverState
{
  float evaltime;

  /* borrowed reference to the 'self' in 'kpy_pydriver_Dict'
   * keep for as long as the same self is used. */
  PyObject *self;

  DriverState() : evaltime(FLT_MAX), self(NULL) {}
};

static DriverState g_pydriver_state_prev;