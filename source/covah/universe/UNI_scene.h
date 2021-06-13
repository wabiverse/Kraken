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
 * @file UNI_scene.h
 * @ingroup UNI
 */

#pragma once

#include "UNI_api.h"

#include <wabi/base/gf/vec2f.h>
#include <wabi/base/gf/vec4d.h>
#include <wabi/base/gf/vec4f.h>

#include <wabi/base/tf/token.h>

#include <wabi/usd/usdGeom/metrics.h>
#include <wabi/usd/usdLux/domeLight.h>

/** Scene display settings, as it pertains to the viewport display. */
struct SceneDisplay {
  /** Light direction for shadows/highlight. */
  float light_direction[3];
  float shadow_shift, shadow_focus;

  /** Method of AA for viewport rendering and image rendering. */
  int viewport_aa;
  int render_aa;

  /** Visual rendering setting. */
  // View3DShading shading;
};

/**
 * ---------------------------------------------------------------------
 *  ::::::::        :       :      :     :    :  :::::: UNI::SCENE
 * ---------------------------------------------------------------------
 */

struct ScenePhoenix {
  /** TODO: Stubbed. */
};

struct SceneArnold {
  /** TODO: Stubbed. */
};

struct SceneRenderman {
  /** TODO: Stubbed. */
};

struct SceneEtcher {
  /** TODO: Stubbed. */
};

struct UnitSettings {
  double meters_per_unit;
  wabi::TfToken up_axis;
  wabi::UsdGeomLinearUnits length_unit;
};

struct SceneRender {
  typedef std::vector<wabi::GfVec4d> ClipPlanesVector;

  wabi::UsdTimeCode current_frame;
  float render_complexity;
  int draw_mode;
  bool show_guides;
  bool show_proxy;
  bool show_render;
  bool force_refresh;
  bool flip_front_facing;
  int cull_style;
  bool enable_id_render;
  bool enable_lighting;
  bool enable_sample_alpha_to_coverage;
  bool apply_render_state;
  bool gamma_correct_colors;
  bool enable_highlight;
  wabi::GfVec4f override_color;
  wabi::GfVec4f wireframe_color;
  float alpha_threshold;
  ClipPlanesVector clip_planes;
  bool enable_scene_materials;
  bool enable_scene_lights;
  bool enable_usd_draw_modes;
  wabi::GfVec4f clear_color;
  wabi::TfToken color_correction_mode;
  int ocio_lut_3d_size;
};

struct Scene {
  /** Unit of measurement this scene adheres to. */
  UnitSettings unit;

  /** Active camera and world on active scene. */
  // wabi::UsdGeomCamera *camera;
  wabi::UsdLuxDomeLight *world;

  /** Viewport scene display / OpenGL / Vulkan. */
  SceneDisplay display;

  SceneRender render;

  /** Hydra Engine render settings. */
  // wabi::EmberRenderSettings ember;

  /** Render Engine settings. */
  ScenePhoenix phoenix;
  SceneArnold arnold;
  SceneRenderman prman;
  SceneEtcher etcher;
};

enum eSceneDrawMode {
  DRAW_POINTS,
  DRAW_WIREFRAME,
  DRAW_WIREFRAME_ON_SURFACE,
  DRAW_SHADED_FLAT,
  DRAW_SHADED_SMOOTH,
  DRAW_GEOM_ONLY,
  DRAW_GEOM_FLAT,
  DRAW_GEOM_SMOOTH
};

enum eSceneCullStyle {
  CULL_STYLE_NO_OPINION,
  CULL_STYLE_NOTHING,
  CULL_STYLE_BACK,
  CULL_STYLE_FRONT,
  CULL_STYLE_BACK_UNLESS_DOUBLE_SIDED,

  CULL_STYLE_COUNT
};

/**
 * GEOMETRY: PRIMITIVES
 * Geometric primitives,
 * which a user can provide
 * to the active stage via
 * shift+a.
 */

enum eScenePrimitives {
  MESH_CUBE = 0,
  MESH_SPHERE,
  MESH_GRID,
  MESH_CONE,
  MESH_CYLINDER,
  MESH_CAPSULE,
  MESH_SUZANNE
};

/**
 * ENGINES: SELECTION
 * Compatible Render Engines
 * enumerated in the same
 * order as they are provided
 * from the plugins directory.
 */

enum eSceneRenderEngine {
  ENGINE_PHOENIX = 0,
  ENGINE_ARNOLD,
  ENGINE_EMBREE,
  ENGINE_RENDERMAN,

  ENGINE_MAX,
};
