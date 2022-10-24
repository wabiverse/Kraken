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
 * Editors.
 * Tools for Artists.
 */

#pragma once

#include "kraken/kraken.h"

#include "ED_defines.h"

#include "USD_view3d.h"

#include <wabi/base/gf/frustum.h>
#include <wabi/base/gf/rotation.h>
#include <wabi/base/gf/vec4d.h>

#include <wabi/base/tf/declarePtrs.h>
#include <wabi/base/tf/hashmap.h>

#include <wabi/base/vt/dictionary.h>

#include <wabi/usd/usd/stage.h>

#include <wabi/imaging/glf/contextCaps.h>

#include <wabi/imaging/hd/driver.h>
#include <wabi/imaging/hd/renderDelegate.h>
#include <wabi/imaging/hd/rendererPlugin.h>

#if WITH_VULKAN
#  include <wabi/imaging/hgi/hgi.h>
#  include <wabi/imaging/hgiVulkan/hgi.h>
#  include <wabi/imaging/hgiVulkan/instance.h>
#endif /* WITH_VULKAN */

#include <wabi/base/gf/vec4f.h>

#include <wabi/usd/usd/timeCode.h>
#include <wabi/usd/usdGeom/camera.h>

#include <wabi/usdImaging/usdImagingGL/engine.h>

#if defined(WABI_STATIC)
#  define VIEW3D_EDITOR_API
#  define VIEW3D_EDITOR_API_TEMPLATE_CLASS(...)
#  define VIEW3D_EDITOR_API_TEMPLATE_STRUCT(...)
#  define VIEW3D_EDITOR_LOCAL
#else
#  if defined(VIEW3D_EDITOR_EXPORTS)
#    define VIEW3D_EDITOR_API ARCH_EXPORT
#    define VIEW3D_EDITOR_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#    define VIEW3D_EDITOR_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#  else
#    define VIEW3D_EDITOR_API ARCH_IMPORT
#    define VIEW3D_EDITOR_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#    define VIEW3D_EDITOR_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#  endif
#  define VIEW3D_EDITOR_LOCAL ARCH_HIDDEN
#endif

VIEW3D_EDITOR_API
void ED_view3d_init_engine(const wabi::SdfPath &root, bool &reset);

VIEW3D_EDITOR_API
void ED_view3d_run(bool *show = NULL);
