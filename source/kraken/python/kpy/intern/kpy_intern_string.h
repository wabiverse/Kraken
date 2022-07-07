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

void kpy_intern_string_init(void);
void kpy_intern_string_exit(void);

extern PyObject *kpy_intern_str___annotations__;
extern PyObject *kpy_intern_str___doc__;
extern PyObject *kpy_intern_str___main__;
extern PyObject *kpy_intern_str___module__;
extern PyObject *kpy_intern_str___name__;
extern PyObject *kpy_intern_str___slots__;
extern PyObject *kpy_intern_str_attr;
extern PyObject *kpy_intern_str_kr_property;
extern PyObject *kpy_intern_str_kr_stage;
extern PyObject *kpy_intern_str_kr_target_properties;
extern PyObject *kpy_intern_str_kpy_types;
extern PyObject *kpy_intern_str_frame;
extern PyObject *kpy_intern_str_properties;
extern PyObject *kpy_intern_str_register;
extern PyObject *kpy_intern_str_self;
extern PyObject *kpy_intern_str_unregister;
