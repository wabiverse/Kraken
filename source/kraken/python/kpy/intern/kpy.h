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

#include <Python.h>

#include "KPY_api.h"
#include "KKE_context.h"

typedef enum eSDFPathForeachFlag
{
  KKE_SDFPATH_FOREACH_PATH_ABSOLUTE = (1 << 0),
  KKE_SDFPATH_FOREACH_PATH_SKIP_LINKED = (1 << 1),
  KKE_SDFPATH_FOREACH_PATH_SKIP_PACKED = (1 << 2),
  KKE_SDFPATH_FOREACH_PATH_RESOLVE_TOKEN = (1 << 3),
  KKE_SDFPATH_TRAVERSE_SKIP_WEAK_REFERENCES = (1 << 5),
  KKE_SDFPATH_FOREACH_PATH_SKIP_MULTIFILE = (1 << 8),
  KKE_SDFPATH_FOREACH_PATH_RELOAD_EDITED = (1 << 9),
} eSDFPathForeachFlag;

struct kContext;

void KPy_init_modules(struct wabi::kContext *C);
extern PyObject *kpy_package_py;

/* kpy_interface_atexit.cpp */
void KPY_atexit_register(void);
void KPY_atexit_unregister(void);
