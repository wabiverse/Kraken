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

#include "KPY_api.h"
#include "KKE_context.h"

/**
 * @note When this struct is passed in as NULL,
 * print errors to the `stdout` and clear.
 */
struct KPy_RunErrInfo {
  /** Brief text, single line (can show this in status bar for e.g.). */
  bool use_single_line_error;

  /** Report with optional prefix (when non-NULL). */
  struct ReportList *reports;
  const char *report_prefix;

  /** Allocated exception text (assign when non-NULL). */
  char **r_string;
};

/**
 * Evaluate `expr` as a number (double).
 *
 * @param C: See @ref common_args.
 * @param imports: See @ref common_args.
 * @param expr: The expression to evaluate.
 * @param err_info: See @ref common_args.
 * @param r_value: The resulting value.
 * @return Success.
 */
bool KPY_run_string_as_number(kContext *C,
                              const char *imports[],
                              const char *expr,
                              struct KPy_RunErrInfo *err_info,
                              double *r_value) ATTR_NONNULL(1, 3, 5);
bool KPY_run_string_exec(struct kContext *C, const char *imports[], const char *expr);
bool KPY_run_string_eval(struct kContext *C, const char *imports[], const char *expr);
