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

#pragma once

/**
 * @file
 * KRAKEN Python.
 * It Bites.
 */

#include <Python.h>

#include "KPY_api.h"

WABI_NAMESPACE_BEGIN

struct kContext;

void KPy_init_modules(struct kContext *C);
extern PyObject *kpy_package_py;

void WABIPy_init_modules(struct kContext *C);
extern PyObject *wabi_package_py;

/* kpy_interface_atexit.cpp */
void KPY_atexit_register(void);
void KPY_atexit_unregister(void);

WABI_NAMESPACE_END