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
#include <frameobject.h>

#include "KLI_utildefines.h"

#include "MEM_guardedalloc.h"

#include "../generic/py_capi_utils.h"
#include "../generic/python_utildefines.h"

#include "kpy_capi_utils.h"

#include "KLI_string.h"
#include "KLI_listbase.h"

#include "KKE_report.h"

bool KPy_errors_to_report_ex(ReportList *reports,
                             const char *error_prefix,
                             const bool use_full,
                             const bool use_location)
{
  PyObject *pystring;

  if (!PyErr_Occurred()) {
    return 1;
  }

  /* less hassle if we allow NULL */
  if (reports == NULL) {
    PyErr_Print();
    PyErr_Clear();
    return 1;
  }

  if (use_full) {
    pystring = PyC_ExceptionBuffer();
  } else {
    pystring = PyC_ExceptionBuffer_Simple();
  }

  if (pystring == NULL) {
    KKE_report(reports, RPT_ERROR, "Unknown py-exception, could not convert");
    return 0;
  }

  if (error_prefix == NULL) {
    /* Not very helpful, better than nothing. */
    error_prefix = "Python";
  }

  if (use_location) {
    const char *filename;
    int lineno;

    PyC_FileAndNum(&filename, &lineno);
    if (filename == NULL) {
      filename = "<unknown location>";
    }

    KKE_reportf(reports,
                RPT_ERROR,
                TIP_("%s: %s\nlocation: %s:%d\n"),
                error_prefix,
                PyUnicode_AsUTF8(pystring),
                filename,
                lineno);

    /* Not exactly needed. Useful for developers tracking down issues. */
    fprintf(stderr,
            TIP_("%s: %s\nlocation: %s:%d\n"),
            error_prefix,
            PyUnicode_AsUTF8(pystring),
            filename,
            lineno);
  } else {
    KKE_reportf(reports, RPT_ERROR, "%s: %s", error_prefix, PyUnicode_AsUTF8(pystring));
  }

  Py_DECREF(pystring);
  return 1;
}

short KPy_reports_to_error(ReportList *reports, PyObject *exception, const bool clear)
{
  char *report_str = nullptr;

  report_str = KKE_reports_string(reports, RPT_ERROR);

  if (clear == true) {
    KKE_reports_clear(reports);
  }

  if (report_str) {
    PyErr_SetString(exception, report_str);
    free(report_str);
  }

  return (report_str == NULL) ? 0 : -1;
}

bool KPy_errors_to_report(ReportList *reports)
{
  return KPy_errors_to_report_ex(reports, NULL, true, true);
}

void KPy_reports_write_stdout(const ReportList *reports, const char *header)
{
  if (header) {
    PySys_WriteStdout("%s\n", header);
  }

  LISTBASE_FOREACH (const Report *, report, &reports->list) {
    PySys_WriteStdout("%s: %s\n", report->typestr, report->message);
  }
}

