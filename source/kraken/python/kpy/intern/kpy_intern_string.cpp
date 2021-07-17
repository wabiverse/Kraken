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

#include <Python.h>

#include "KPY_api.h"

#include "kpy_intern_string.h"

#include "KLI_assert.h"
#include "KLI_utildefines.h"

#include <wabi/base/tf/iterator.h>

WABI_NAMESPACE_BEGIN

static PyObject *kpy_intern_str_arr[16];

PyObject *kpy_intern_str___annotations__;
PyObject *kpy_intern_str___doc__;
PyObject *kpy_intern_str___main__;
PyObject *kpy_intern_str___module__;
PyObject *kpy_intern_str___name__;
PyObject *kpy_intern_str___slots__;
PyObject *kpy_intern_str_attr;
PyObject *kpy_intern_str_kr_property;
PyObject *kpy_intern_str_kr_uni;
PyObject *kpy_intern_str_kr_target_properties;
PyObject *kpy_intern_str_kpy_types;
PyObject *kpy_intern_str_frame;
PyObject *kpy_intern_str_properties;
PyObject *kpy_intern_str_register;
PyObject *kpy_intern_str_self;
PyObject *kpy_intern_str_unregister;

void kpy_intern_string_init(void)
{
  uint i = 0;

#define KPY_INTERN_STR(var, str) \
  { \
    var = kpy_intern_str_arr[i++] = PyUnicode_FromString(str); \
  } \
  (void)0

  KPY_INTERN_STR(kpy_intern_str___annotations__, "__annotations__");
  KPY_INTERN_STR(kpy_intern_str___doc__, "__doc__");
  KPY_INTERN_STR(kpy_intern_str___main__, "__main__");
  KPY_INTERN_STR(kpy_intern_str___module__, "__module__");
  KPY_INTERN_STR(kpy_intern_str___name__, "__name__");
  KPY_INTERN_STR(kpy_intern_str___slots__, "__slots__");
  KPY_INTERN_STR(kpy_intern_str_attr, "attr");
  KPY_INTERN_STR(kpy_intern_str_kr_property, "kr_property");
  KPY_INTERN_STR(kpy_intern_str_kr_uni, "kr_uni");
  KPY_INTERN_STR(kpy_intern_str_kr_target_properties, "kr_target_properties");
  KPY_INTERN_STR(kpy_intern_str_kpy_types, "kpy.types");
  KPY_INTERN_STR(kpy_intern_str_frame, "frame");
  KPY_INTERN_STR(kpy_intern_str_properties, "properties");
  KPY_INTERN_STR(kpy_intern_str_register, "register");
  KPY_INTERN_STR(kpy_intern_str_self, "self");
  KPY_INTERN_STR(kpy_intern_str_unregister, "unregister");

#undef KPY_INTERN_STR

  KLI_assert(i == TfArraySize(kpy_intern_str_arr));
}

void kpy_intern_string_exit(void)
{
  uint i = TfArraySize(kpy_intern_str_arr);
  while (i--) {
    Py_DECREF(kpy_intern_str_arr[i]);
  }
}

WABI_NAMESPACE_END