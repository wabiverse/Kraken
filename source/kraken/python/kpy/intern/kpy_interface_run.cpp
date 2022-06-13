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

#include "KKE_appdir.h"

#include "KPY_api.h"
#include "KPY_extern_run.h"

#include "kpy.h"
#include "kpy_capi_utils.h"
#include "kpy_interface.h"
#include "kpy_intern_string.h"
#include "kpy_path.h"
#include "kpy_uni.h"

WABI_NAMESPACE_BEGIN

/**
 * @param mode: Passed to #PyRun_String, matches Python's
 * `compile` functions mode argument. #Py_eval_input for
 * `eval`, #Py_file_input for `exec`. */
static bool kpy_run_string_impl(kContext *C,
                                const char *imports[],
                                const char *expr,
                                const int mode)
{
  KLI_assert(expr);
  PyGILState_STATE gilstate;
  PyObject *main_mod = NULL;
  PyObject *py_dict, *retval;
  bool ok = true;

  if (expr[0] == '\0') {
    return ok;
  }

  kpy_context_set(C, &gilstate);

  PyC_MainModule_Backup(&main_mod);

  py_dict = PyC_DefaultNameSpace("<kraken string>");

  if (imports && (!PyC_NameSpace_ImportArray(py_dict, imports))) {
    Py_DECREF(py_dict);
    retval = NULL;
  } else {
    retval = PyRun_String(expr, mode, py_dict, py_dict);
  }

  if (retval == NULL) {
    ok = false;
    // KPy_errors_to_report(CTX_wm_reports(C));
  } else {
    Py_DECREF(retval);
  }

  PyC_MainModule_Restore(main_mod);

  kpy_context_clear(C, &gilstate);

  return ok;
}

/**
 * Run an expression, matches: `exec(compile(..., "eval"))` */
bool KPY_run_string_eval(kContext *C, const char *imports[], const char *expr)
{
  return kpy_run_string_impl(C, imports, expr, Py_eval_input);
}

/**
 * Run an entire script, matches: `exec(compile(..., "exec"))` */
bool KPY_run_string_exec(kContext *C, const char *imports[], const char *expr)
{
  return kpy_run_string_impl(C, imports, expr, Py_file_input);
}

WABI_NAMESPACE_END