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
 * @file Draw.
 * Spontaneous Expression.
 * 
 * This is the Render Functions used by Realtime engines to draw.
 */

#include "KLI_listbase.h"
#include "KLI_math_matrix.h"
#include "KLI_math_vector.h"
#include "KLI_string.h"

#include "KKE_context.h"
// #include "KKE_layer.h"
#include "KKE_material.h"
// #include "KKE_scene.h"

// #include "USD_light_types.h"
// #include "USD_material_types.h"
// #include "USD_object_types.h"
#include "USD_scene_types.h"
// #include "USD_world_types.h"

#include "GPU_framebuffer.h"
#include "GPU_material.h"
#include "GPU_primitive.h"
#include "GPU_shader.h"
#include "GPU_storage_buffer.h"
#include "GPU_texture.h"
#include "GPU_uniform_buffer.h"

// #include "draw_cache.h"
// #include "draw_common.h"
// #include "draw_view.h"

// #include "draw_debug.h"
// #include "draw_manager_profiling.h"
#include "draw_state.h"
// #include "draw_view_data.h"

#include "MEM_guardedalloc.h"

#include "RE_engine.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Uncomment to track unused resource bindings. */
// #define DRW_UNUSED_RESOURCE_TRACKING

#ifdef DRW_UNUSED_RESOURCE_TRACKING
#  define DRW_DEBUG_FILE_LINE_ARGS , const char *file, int line
#else
#  define DRW_DEBUG_FILE_LINE_ARGS
#endif

struct GPUBatch;
struct GPUMaterial;
struct GPUShader;
struct GPUTexture;
struct GPUUniformBuf;
struct Object;
struct ParticleSystem;
struct RenderEngineType;
struct kContext;
struct rcti;

typedef struct DRWCallBuffer DRWCallBuffer;
typedef struct DRWInterface DRWInterface;
typedef struct DRWPass DRWPass;
typedef struct DRWShaderLibrary DRWShaderLibrary;
typedef struct DRWShadingGroup DRWShadingGroup;
typedef struct DRWUniform DRWUniform;
typedef struct DRWView DRWView;

/* TODO: Put it somewhere else? */
typedef struct BoundSphere {
  float center[3], radius;
} BoundSphere;

/* declare members as empty (unused) */
typedef char DRWViewportEmptyList;

#define DRW_VIEWPORT_LIST_SIZE(list) \
  (sizeof(list) == sizeof(DRWViewportEmptyList) ? 0 : (sizeof(list) / sizeof(void *)))

/* Unused members must be either pass list or 'char *' when not used. */
#define DRW_VIEWPORT_DATA_SIZE(ty) \
  { \
    DRW_VIEWPORT_LIST_SIZE(*(((ty *)NULL)->fbl)), DRW_VIEWPORT_LIST_SIZE(*(((ty *)NULL)->txl)), \
        DRW_VIEWPORT_LIST_SIZE(*(((ty *)NULL)->psl)), \
        DRW_VIEWPORT_LIST_SIZE(*(((ty *)NULL)->stl)), \
  }

typedef struct DrawEngineDataSize {
  int fbl_len;
  int txl_len;
  int psl_len;
  int stl_len;
} DrawEngineDataSize;

typedef struct DrawEngineType {
  struct DrawEngineType *next, *prev;

  char idname[32];

  const DrawEngineDataSize *vedata_size;

  void (*engine_init)(void *vedata);
  void (*engine_free)(void);

  void (*instance_free)(void *instance_data);

  void (*cache_init)(void *vedata);
  void (*cache_populate)(void *vedata, struct Object *ob);
  void (*cache_finish)(void *vedata);

  void (*draw_scene)(void *vedata);

  void (*view_update)(void *vedata);
  void (*id_update)(void *vedata, struct ID *id);

  void (*render_to_image)(void *vedata,
                          struct RenderEngine *engine,
                          struct RenderLayer *layer,
                          const struct rcti *rect);
  void (*store_metadata)(void *vedata, struct RenderResult *render_result);
} DrawEngineType;

/* Textures */
typedef enum {
  DRW_TEX_FILTER = (1 << 0),
  DRW_TEX_WRAP = (1 << 1),
  DRW_TEX_COMPARE = (1 << 2),
  DRW_TEX_MIPMAP = (1 << 3),
} DRWTextureFlag;


/* Avoid too many lookups while drawing */
typedef struct DRWContextState {

  struct ARegion *region;       /* 'CTX_wm_region(C)' */
  struct RegionView3D *rv3d;    /* 'CTX_wm_region_view3d(C)' */
  struct View3D *v3d;           /* 'CTX_wm_view3d(C)' */
  struct SpaceLink *space_data; /* 'CTX_wm_space_data(C)' */

  struct Scene *scene;          /* 'CTX_data_scene(C)' */
  struct ViewLayer *view_layer; /* 'CTX_data_view_layer(C)' */

  /* Use 'object_edit' for edit-mode */
  struct Object *obact;

  struct RenderEngineType *engine_type;

  struct Hydra *hydra;

  struct TaskGraph *task_graph;

  eObjectMode object_mode;

  eGPUShaderConfig sh_cfg;

  /** Last resort (some functions take this as an arg so we can't easily avoid).
   * May be NULL when used for selection or depth buffer. */
  const struct kContext *evil_C;

  /* ---- */

  /* Cache: initialized by 'drw_context_state_init'. */
  struct Object *object_pose;
  struct Object *object_edit;

} DRWContextState;

