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
 * GPU.
 * Pixel Magic.
 *
 * Intermediate node graph for generating GLSL shaders.
 */

#include "MEM_guardedalloc.h"

#include "KLI_dynstr.h"
#include "KLI_string.h"

#include "GPU_platform.h"

#include "gpu_platform_private.hh"

/* -------------------------------------------------------------------- */
/** \name GPUPlatformGlobal
 * \{ */

namespace kraken::gpu
{

  GPUPlatformGlobal GPG;

  static char *create_key(eGPUSupportLevel support_level,
                          const char *vendor,
                          const char *renderer,
                          const char *version)
  {
    DynStr *ds = KLI_dynstr_new();
    KLI_dynstr_appendf(ds, "{%s/%s/%s}=", vendor, renderer, version);
    if (support_level == GPU_SUPPORT_LEVEL_SUPPORTED) {
      KLI_dynstr_append(ds, "SUPPORTED");
    } else if (support_level == GPU_SUPPORT_LEVEL_LIMITED) {
      KLI_dynstr_append(ds, "LIMITED");
    } else {
      KLI_dynstr_append(ds, "UNSUPPORTED");
    }

    char *support_key = KLI_dynstr_get_cstring(ds);
    KLI_dynstr_free(ds);
    KLI_str_replace_char(support_key, '\n', ' ');
    KLI_str_replace_char(support_key, '\r', ' ');
    return support_key;
  }

  static char *create_gpu_name(const char *vendor, const char *renderer, const char *version)
  {
    DynStr *ds = KLI_dynstr_new();
    KLI_dynstr_appendf(ds, "%s %s %s", vendor, renderer, version);

    char *gpu_name = KLI_dynstr_get_cstring(ds);
    KLI_dynstr_free(ds);
    KLI_str_replace_char(gpu_name, '\n', ' ');
    KLI_str_replace_char(gpu_name, '\r', ' ');
    return gpu_name;
  }

  void GPUPlatformGlobal::init(eGPUDeviceType gpu_device,
                               eGPUOSType os_type,
                               eGPUDriverType driver_type,
                               eGPUSupportLevel gpu_support_level,
                               eGPUBackendType backend,
                               const char *vendor_str,
                               const char *renderer_str,
                               const char *version_str)
  {
    this->clear();

    this->initialized = true;

    this->device = gpu_device;
    this->os = os_type;
    this->driver = driver_type;
    this->support_level = gpu_support_level;

    const char *vendor = vendor_str ? vendor_str : "UNKNOWN";
    const char *renderer = renderer_str ? renderer_str : "UNKNOWN";
    const char *version = version_str ? version_str : "UNKNOWN";

    this->vendor = KLI_strdup(vendor);
    this->renderer = KLI_strdup(renderer);
    this->version = KLI_strdup(version);
    this->support_key = create_key(gpu_support_level, vendor, renderer, version);
    this->gpu_name = create_gpu_name(vendor, renderer, version);
    this->backend = backend;
  }

  void GPUPlatformGlobal::clear()
  {
    MEM_SAFE_FREE(vendor);
    MEM_SAFE_FREE(renderer);
    MEM_SAFE_FREE(version);
    MEM_SAFE_FREE(support_key);
    MEM_SAFE_FREE(gpu_name);
    initialized = false;
  }

}  // namespace kraken::gpu

/** \} */

/* -------------------------------------------------------------------- */
/** \name C-API
 * \{ */

using namespace kraken::gpu;

eGPUSupportLevel GPU_platform_support_level()
{
  KLI_assert(GPG.initialized);
  return GPG.support_level;
}

const char *GPU_platform_vendor()
{
  KLI_assert(GPG.initialized);
  return GPG.vendor;
}

const char *GPU_platform_renderer()
{
  KLI_assert(GPG.initialized);
  return GPG.renderer;
}

const char *GPU_platform_version()
{
  KLI_assert(GPG.initialized);
  return GPG.version;
}

const char *GPU_platform_support_level_key()
{
  KLI_assert(GPG.initialized);
  return GPG.support_key;
}

const char *GPU_platform_gpu_name()
{
  KLI_assert(GPG.initialized);
  return GPG.gpu_name;
}

bool GPU_type_matches(eGPUDeviceType device, eGPUOSType os, eGPUDriverType driver)
{
  return GPU_type_matches_ex(device, os, driver, GPU_BACKEND_ANY);
}

bool GPU_type_matches_ex(eGPUDeviceType device,
                         eGPUOSType os,
                         eGPUDriverType driver,
                         eGPUBackendType backend)
{
  KLI_assert(GPG.initialized);
  return (GPG.device & device) && (GPG.os & os) && (GPG.driver & driver) &&
         (GPG.backend & backend);
}

/** \} */
