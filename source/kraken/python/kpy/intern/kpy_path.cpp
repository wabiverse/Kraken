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

#include "kpy_path.h"

WABI_NAMESPACE_BEGIN

/*----------------------------MODULE INIT-------------------------*/
static struct PyModuleDef _kpy_path_module_def = {
  PyModuleDef_HEAD_INIT,
  "_kpy_path", /* m_name */
  NULL,        /* m_doc */
  0,           /* m_size */
  NULL,        /* m_methods */
  NULL,        /* m_reload */
  NULL,        /* m_traverse */
  NULL,        /* m_clear */
  NULL,        /* m_free */
};

PyObject *KPyInit__kpy_path(void)
{
  PyObject *submodule;

  submodule = PyModule_Create(&_kpy_path_module_def);

  return submodule;
}

WABI_NAMESPACE_END