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

#define VIEW3D_MAXNAME 64

/** Figure out why this is the magical timecode multiplier value. */
#define MAGIC_TIMECODE_MULTIPLIER 0.2

KRAKEN_NAMESPACE_BEGIN

/** Use with #View3D.shading.mode */
enum eView3DShadingMode
{
  VIEW3D_MODE_WIREFRAME = 0,
  VIEW3D_MODE_SOLID,
  VIEW3D_MODE_SHADED
};

struct View3DShading
{
  /** Shading type (WIREFRAME, ..). */
  eView3DShadingMode mode;

  /** Viewport background color. */
  wabi::GfVec4f background_color;

  /** HDR dome light texture settings. */
  char studiolight_texture[VIEW3D_MAXNAME];
  float studiolight_rot_z;
  float studiolight_background;
  float studiolight_intensity;
  float studiolight_blur;

  /** Selection outline color. */
  wabi::GfVec4f object_outline_color;

  /** Alpha transparency for xray'd objects. */
  float xray_alpha;

  int render_pass;
  char aov_name[VIEW3D_MAXNAME];
};

/** Use with #View3D.status */
enum eView3DStatusFlag
{
  VIEW3D_SUCCESS = 0,
  VIEW3D_ERROR,
  VIEW3D_UPDATE,
  VIEW3D_IDLE,
  VIEW3D_INIT,
};

struct View3DStatus
{
  /** Hydra graphics interface status flag. */
  eView3DStatusFlag hgi_flag;
  /** Render engine selection status flag. */
  eView3DStatusFlag engine_flag;
  /** Viewport camera status flag. */
  eView3DStatusFlag camera_flag;
  /** Viewport hydra export status flag. */
  eView3DStatusFlag export_flag;
  /** Hydra engine settings status flag. */
  eView3DStatusFlag settings_flag;
  /** Viewport Animation status flag. */
  eView3DStatusFlag animation_flag;
  /** Viewport redraw status flag. */
  eView3DStatusFlag redraw_flag;

  /** Animation timecode. */
  wabi::UsdTimeCode timecode;
};

struct View3DOverlay
{
  /** Edit mode settings. */
  int edit_flag;
  float normals_length;
  float backwire_opacity;

  /** Paint mode settings. */
  int paint_flag;

  /** Weight paint mode settings. */
  int weight_paint_flag;

  /** Alpha for texture, weight, vertex paint overlay. */
  float texture_paint_mode_opacity;
  float vertex_paint_mode_opacity;
  float weight_paint_mode_opacity;
  float sculpt_mode_mask_opacity;
  float sculpt_mode_face_sets_opacity;

  /** Skel edit/pose mode settings. */
  float xray_alpha_bone;

  /** Darken Inactive. */
  float fade_alpha;

  /** Other settings. */
  float wireframe_threshold;
  float wireframe_opacity;

  /** Etcher brush settings. */
  float etcher_paper_opacity;
  float etcher_grid_opacity;
  float etcher_fade_layer;

  /** Factor for mixing vertex paint with original color */
  float etcher_vertex_paint_opacity;
};

struct View3D
{
  /** Viewport camera settings. */
  wabi::UsdGeomCamera camera;
  float lens;
  float clip_start, clip_end;

  /** Animation playback. */
  bool anim_playback;
  double anim_start, anim_end;

  /** Show / hide 3D gizmos. */
  bool gizmo_show_geo;
  bool gizmo_show_skel;
  bool gizmo_show_xform;
  bool gizmo_show_lux;
  bool gizmo_show_camera;

  /** Viewport gridline settings. */
  int gridlines;
  int gridsubdiv;

  /** Viewport display settings. */
  View3DShading shading;
  View3DOverlay overlay;

  /** Hydra update status. */
  View3DStatus status;

  /** Hydra Engine Params. */
  wabi::UsdImagingGLRenderParams render_params;
};

VIEW3D_EDITOR_API
void ED_view3d_init_engine(const wabi::SdfPath &root, bool &reset);

VIEW3D_EDITOR_API
void ED_view3d_run(bool *show = NULL);

KRAKEN_NAMESPACE_END