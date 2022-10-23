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

#pragma once

/**
 * @file
 * KRAKEN Python.
 * It Bites.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct EnumPropertyItem;
struct ReportList;

/* Kpy ----  */

/* error reporting */
short KPy_reports_to_error(struct ReportList *reports, PyObject *exception, const bool clear);
/**
 * A version of #KKE_report_write_file_fp that uses Python's stdout.
 */
void KPy_reports_write_stdout(const struct ReportList *reports, const char *header);
bool KPy_errors_to_report_ex(struct ReportList *reports,
                             const char *error_prefix,
                             const bool use_full,
                             const bool use_location);
/**
 * @param reports: When set, an error will be added to this report, when NULL, print the error.
 *
 * @note Unless the caller handles printing the reports (or reports is NULL) it's best to ensure
 * the output is printed to the `stdout/stderr`:
 * @code{.cc}
 * KPy_errors_to_report(reports);
 * if (!KKE_reports_print_test(reports)) {
 *   KKE_reports_print(reports);
 * }
 * @endcode
 *
 * @note The caller is responsible for clearing the error (see #PyErr_Clear).
 */
bool KPy_errors_to_report(struct ReportList *reports);

struct kContext *KPY_context_get(void);

extern void kpy_context_set(struct kContext *C, PyGILState_STATE *gilstate);
/**
 * Context should be used but not now because it causes some bugs.
 */
extern void kpy_context_clear(struct kContext *C, const PyGILState_STATE *gilstate);

#ifdef __cplusplus
}
#endif
