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
 * @ingroup kpygpu
 * KRAKEN Python GPU.
 * It Bites. It Draws.
 *
 * This file defines the gpu.matrix stack API.
 *
 * Experimental Python API, not considered public yet (called '_gpu'),
 * we may re-expose as public later.
 *
 * - Use `kpygpu_` for local API.
 * - Use `KPyGPU` for public API.
 */

#include <Python.h>

#include "KLI_utildefines.h"

// #include "gpu_py_capabilities.h"
#include "gpu_py_matrix.h"
// #include "gpu_py_platform.h"
// #include "gpu_py_select.h"
// #include "gpu_py_state.h"
// #include "gpu_py_types.h"

#include "gpu_py_api.h" /* Own include. */

/* -------------------------------------------------------------------- */
/** @name GPU Module
 * @{ */

PyDoc_STRVAR(pygpu_doc,
             "This module provides Python wrappers for the GPU implementation in Kraken.\n"
             "Some higher level functions can be found in the `gpu_extras` module.");
static struct PyModuleDef pygpu_module_def = {
    PyModuleDef_HEAD_INIT,
    .m_name = "gpu",
    .m_doc = pygpu_doc,
};

PyObject *KPyInit_gpu(void)
{
  PyObject *sys_modules = PyImport_GetModuleDict();
  PyObject *submodule;
  PyObject *mod;

  mod = PyModule_Create(&pygpu_module_def);

  // PyModule_AddObject(mod, "types", (submodule = kpygpu_types_init()));
  // PyDict_SetItem(sys_modules, PyModule_GetNameObject(submodule), submodule);

  // PyModule_AddObject(mod, "capabilities", (submodule = kpygpu_capabilities_init()));
  // PyDict_SetItem(sys_modules, PyModule_GetNameObject(submodule), submodule);

  PyModule_AddObject(mod, "matrix", (submodule = kpygpu_matrix_init()));
  PyDict_SetItem(sys_modules, PyModule_GetNameObject(submodule), submodule);

  // PyModule_AddObject(mod, "platform", (submodule = kpygpu_platform_init()));
  // PyDict_SetItem(sys_modules, PyModule_GetNameObject(submodule), submodule);

  // PyModule_AddObject(mod, "select", (submodule = kpygpu_select_init()));
  // PyDict_SetItem(sys_modules, PyModule_GetNameObject(submodule), submodule);

  // PyModule_AddObject(mod, "shader", (submodule = kpygpu_shader_init()));
  // PyDict_SetItem(sys_modules, PyModule_GetNameObject(submodule), submodule);

  // PyModule_AddObject(mod, "state", (submodule = kpygpu_state_init()));
  // PyDict_SetItem(sys_modules, PyModule_GetNameObject(submodule), submodule);

  // PyModule_AddObject(mod, "texture", (submodule = kpygpu_texture_init()));
  // PyDict_SetItem(sys_modules, PyModule_GetNameObject(submodule), submodule);

  return mod;
}

/** @} */
