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

#include "KKE_appdir.h"
#include "KKE_appdir.hh"
#include "KKE_context.h"
#include "KKE_report.h"

#include "../generic/py_capi_utils.h"
#include "../generic/python_utildefines.h"

#include "KPY_api.h"
#include "KPY_extern_run.h"

#include "kpy.h"
#include "kpy_capi_utils.h"
#include "kpy_interface.h"
#include "kpy_intern_string.h"
#include "kpy_path.h"
#include "kpy_prim.h"

#include <boost/python.hpp>
#include <boost/python/overloads.hpp>

using namespace boost::python;

KRAKEN_NAMESPACE_USING

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

    ReportList reports;
    KKE_reports_init(&reports, RPT_STORE);
    KPy_errors_to_report(&reports);
    PyErr_Clear();

    /* Ensure the reports are printed. */
    if (!KKE_reports_print_test(&reports, RPT_ERROR)) {
      KKE_reports_print(&reports, RPT_ERROR);
    }

  } else {
    Py_DECREF(retval);
  }

  PyC_MainModule_Restore(main_mod);

  kpy_context_clear(C, &gilstate);

  return ok;
}

/* -------------------------------------------------------------------- */
/** \name Run Python & Evaluate Utilities
 *
 * Return values as plain C types, useful to run Python scripts
 * in code that doesn't deal with Python data-types.
 * \{ */

static void run_string_handle_error(struct KPy_RunErrInfo *err_info)
{
  if (err_info == NULL) {
    PyErr_Print();
    PyErr_Clear();
    return;
  }

  /* Signal to do nothing. */
  if (!(err_info->reports || err_info->r_string)) {
    PyErr_Clear();
    return;
  }

  PyObject *py_err_str = err_info->use_single_line_error ? PyC_ExceptionBuffer_Simple() :
                                                           PyC_ExceptionBuffer();
  const char *err_str = py_err_str ? PyUnicode_AsUTF8(py_err_str) : "Unable to extract exception";
  PyErr_Clear();

  if (err_info->reports != NULL) {
    if (err_info->report_prefix) {
      KKE_reportf(err_info->reports, RPT_ERROR, "%s: %s", err_info->report_prefix, err_str);
    }
    else {
      KKE_report(err_info->reports, RPT_ERROR, err_str);
    }
  }

  /* Print the reports if they were not printed already. */
  if ((err_info->reports == NULL) || !KKE_reports_print_test(err_info->reports, RPT_ERROR)) {
    if (err_info->report_prefix) {
      fprintf(stderr, "%s: ", err_info->report_prefix);
    }
    fprintf(stderr, "%s\n", err_str);
  }

  if (err_info->r_string != NULL) {
    *err_info->r_string = KLI_strdup(err_str);
  }

  Py_XDECREF(py_err_str);
}

bool KPY_run_string_as_number(struct kContext *C,
                              const char *imports[],
                              const char *expr,
                              struct KPy_RunErrInfo *err_info,
                              double *r_value) ATTR_NONNULL(1, 3, 5)
{
  PyGILState_STATE gilstate;
  bool ok = true;

  if (expr[0] == '\0') {
    *r_value = 0.0;
    return ok;
  }

  kpy_context_set(C, &gilstate);

  ok = PyC_RunString_AsNumber(imports, expr, "<expr as number>", r_value);

  if (ok == false) {
    run_string_handle_error(err_info);
  }

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
