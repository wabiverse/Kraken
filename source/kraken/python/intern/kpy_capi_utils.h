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

bool KPy_errors_to_report(struct ReportList *reports);
short KPy_reports_to_error(struct ReportList *reports, PyObject *exception, const bool clear);
void KPy_reports_write_stdout(const struct ReportList *reports, const char *header);

extern void kpy_context_set(struct kContext *C, PyGILState_STATE *gilstate);
extern void kpy_context_clear(struct kContext *C, const PyGILState_STATE *gilstate);

#ifdef __cplusplus
}
#endif
