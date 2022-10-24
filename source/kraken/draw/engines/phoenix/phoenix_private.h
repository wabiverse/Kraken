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
 * The Phoenix Render Engine.
 * The OpenSubdiv-based real-time render engine of the 21st century.
 */

#include "DRW_render.h"

#include "KLI_bitmap.h"

#include "GPU_framebuffer.h"
#include "GPU_viewport.h"

#include "USD_object_types.h"
#include "USD_view3d.h"

#include <wabi/imaging/hd/bufferArray.h>
#include <wabi/imaging/hd/resourceRegistry.h>

#include "phoenix_api.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PhoenixShadowCasterBuffer;
struct GPUFrameBuffer;
struct Object;
struct RenderLayer;

extern struct DrawEngineType draw_engine_phoenix_type;

/* Minimum UBO is 16384 bytes */
#define MAX_PROBE 128 /* TODO: find size by dividing UBO max size by probe data size. */
#define MAX_GRID 64   /* TODO: find size by dividing UBO max size by grid data size. */
#define MAX_PLANAR 16 /* TODO: find size by dividing UBO max size by grid data size. */
#define MAX_LIGHT 128 /* TODO: find size by dividing UBO max size by light data size. */
#define MAX_CASCADE_NUM 4
#define MAX_SHADOW 128
#define MAX_SHADOW_CASCADE 8
#define MAX_SHADOW_CUBE (MAX_SHADOW - MAX_CASCADE_NUM * MAX_SHADOW_CASCADE)
#define MAX_BLOOM_STEP 16
#define MAX_AOVS 64

/* Special value chosen to not be altered by depth of field sample count. */
#define TAA_MAX_SAMPLE 10000926

// #define DEBUG_SHADOW_DISTRIBUTION

/* Only define one of these. */
// #define IRRADIANCE_SH_L2
#define IRRADIANCE_HL2

#if defined(IRRADIANCE_SH_L2)
#  define SHADER_IRRADIANCE "#define IRRADIANCE_SH_L2\n"
#elif defined(IRRADIANCE_HL2)
#  define SHADER_IRRADIANCE "#define IRRADIANCE_HL2\n"
#endif

/* Macro causes over indentation. */
/* clang-format off */
#define SHADER_DEFINES \
  "#define PHOENIX_ENGINE\n" \
  "#define MAX_PROBE " STRINGIFY(MAX_PROBE) "\n" \
  "#define MAX_GRID " STRINGIFY(MAX_GRID) "\n" \
  "#define MAX_PLANAR " STRINGIFY(MAX_PLANAR) "\n" \
  "#define MAX_LIGHT " STRINGIFY(MAX_LIGHT) "\n" \
  "#define MAX_SHADOW " STRINGIFY(MAX_SHADOW) "\n" \
  "#define MAX_SHADOW_CUBE " STRINGIFY(MAX_SHADOW_CUBE) "\n" \
  "#define MAX_SHADOW_CASCADE " STRINGIFY(MAX_SHADOW_CASCADE) "\n" \
  "#define MAX_CASCADE_NUM " STRINGIFY(MAX_CASCADE_NUM) "\n" \
  SHADER_IRRADIANCE
/* clang-format on */

#define PHOENIX_PROBE_MAX min_ii(MAX_PROBE, GPU_max_texture_layers() / 6)
#define PHOENIX_VELOCITY_TILE_SIZE 32
#define USE_VOLUME_OPTI (GPU_shader_image_load_store_support())

#define SWAP_DOUBLE_BUFFERS()                                                         \
  {                                                                                   \
    if (effects->swap_double_buffer) {                                                \
      SWAP(struct GPUFrameBuffer *, fbl->main_fb, fbl->double_buffer_fb);             \
      SWAP(struct GPUFrameBuffer *, fbl->main_color_fb, fbl->double_buffer_color_fb); \
      SWAP(GPUTexture *, txl->color, txl->color_double_buffer);                       \
      effects->swap_double_buffer = false;                                            \
    }                                                                                 \
  }                                                                                   \
  ((void)0)

#define SWAP_BUFFERS()                                    \
  {                                                       \
    if (effects->target_buffer == fbl->effect_color_fb) { \
      SWAP_DOUBLE_BUFFERS();                              \
      effects->source_buffer = txl->color_post;           \
      effects->target_buffer = fbl->main_color_fb;        \
    } else {                                              \
      SWAP_DOUBLE_BUFFERS();                              \
      effects->source_buffer = txl->color;                \
      effects->target_buffer = fbl->effect_color_fb;      \
    }                                                     \
  }                                                       \
  ((void)0)

#define SWAP_BUFFERS_TAA()                                                            \
  {                                                                                   \
    if (effects->target_buffer == fbl->effect_color_fb) {                             \
      SWAP(struct GPUFrameBuffer *, fbl->effect_fb, fbl->taa_history_fb);             \
      SWAP(struct GPUFrameBuffer *, fbl->effect_color_fb, fbl->taa_history_color_fb); \
      SWAP(GPUTexture *, txl->color_post, txl->taa_history);                          \
      effects->source_buffer = txl->taa_history;                                      \
      effects->target_buffer = fbl->effect_color_fb;                                  \
    } else {                                                                          \
      SWAP(struct GPUFrameBuffer *, fbl->main_fb, fbl->taa_history_fb);               \
      SWAP(struct GPUFrameBuffer *, fbl->main_color_fb, fbl->taa_history_color_fb);   \
      SWAP(GPUTexture *, txl->color, txl->taa_history);                               \
      effects->source_buffer = txl->taa_history;                                      \
      effects->target_buffer = fbl->main_color_fb;                                    \
    }                                                                                 \
  }                                                                                   \
  ((void)0)

KLI_INLINE bool eevee_hdri_preview_overlay_enabled(const View3D *v3d)
{
  /* Only show the HDRI Preview in Shading Preview in the Viewport. */
  if (v3d == NULL || v3d->shading.type != OB_MATERIAL) {
    return false;
  }

  /* Only show the HDRI Preview when viewing the Combined render pass */
  if (v3d->shading.render_pass != SCE_PASS_COMBINED) {
    return false;
  }

  return ((v3d->flag2 & V3D_HIDE_OVERLAYS) == 0) && (v3d->overlay.flag & V3D_OVERLAY_LOOK_DEV);
}

#define USE_SCENE_LIGHT(v3d)                                                                 \
  ((!v3d) ||                                                                                 \
   ((v3d->shading.type == OB_MATERIAL) && (v3d->shading.flag & V3D_SHADING_SCENE_LIGHTS)) || \
   ((v3d->shading.type == OB_RENDER) && (v3d->shading.flag & V3D_SHADING_SCENE_LIGHTS_RENDER)))
#define LOOK_DEV_STUDIO_LIGHT_ENABLED(v3d)                             \
  ((v3d) && (((v3d->shading.type == OB_MATERIAL) &&                    \
              ((v3d->shading.flag & V3D_SHADING_SCENE_WORLD) == 0)) || \
             ((v3d->shading.type == OB_RENDER) &&                      \
              ((v3d->shading.flag & V3D_SHADING_SCENE_WORLD_RENDER) == 0))))

#define MIN_CUBE_LOD_LEVEL 3
#define MAX_SCREEN_BUFFERS_LOD_LEVEL 6

/* All the renderpasses that use the GPUMaterial for accumulation */
#define PHOENIX_RENDERPASSES_MATERIAL                                       \
  (PHOENIX_RENDER_PASS_EMIT | PHOENIX_RENDER_PASS_DIFFUSE_COLOR |           \
   PHOENIX_RENDER_PASS_DIFFUSE_LIGHT | PHOENIX_RENDER_PASS_SPECULAR_COLOR | \
   PHOENIX_RENDER_PASS_SPECULAR_LIGHT | PHOENIX_RENDER_PASS_ENVIRONMENT |   \
   PHOENIX_RENDER_PASS_AOV)
#define PHOENIX_AOV_HASH_ALL -1
#define PHOENIX_AOV_HASH_COLOR_TYPE_MASK 1
#define MAX_CRYPTOMATTE_LAYERS 3

/* Material shader variations */
enum
{
  VAR_MAT_MESH = (1 << 0),
  VAR_MAT_VOLUME = (1 << 1),
  VAR_MAT_HAIR = (1 << 2),
  VAR_MAT_POINTCLOUD = (1 << 3),
  VAR_MAT_BLEND = (1 << 4),
  VAR_MAT_LOOKDEV = (1 << 5),
  VAR_MAT_HOLDOUT = (1 << 6),
  VAR_MAT_HASH = (1 << 7),
  VAR_MAT_DEPTH = (1 << 8),
  VAR_MAT_REFRACT = (1 << 9),
  VAR_WORLD_BACKGROUND = (1 << 10),
  VAR_WORLD_PROBE = (1 << 11),
  VAR_WORLD_VOLUME = (1 << 12),
  VAR_DEFAULT = (1 << 13),
};

/* Material shader cache keys */
enum
{
  /* HACK: This assumes the struct GPUShader will never be smaller than our variations.
   * This allow us to only keep one #GHash and avoid bigger keys comparisons/hashing.
   * We combine the #GPUShader pointer with the key. */
  KEY_CULL = (1 << 0),
  KEY_REFRACT = (1 << 1),
  KEY_HAIR = (1 << 2),
  KEY_SHADOW = (1 << 3),
};

/* DOF Gather pass shader variations */
typedef enum PHOENIX_DofGatherPass
{
  DOF_GATHER_FOREGROUND = 0,
  DOF_GATHER_BACKGROUND = 1,
  DOF_GATHER_HOLEFILL = 2,

  DOF_GATHER_MAX_PASS,
} PHOENIX_DofGatherPass;

#define DOF_TILE_DIVISOR 16
#define DOF_BOKEH_LUT_SIZE 32
#define DOF_GATHER_RING_COUNT 5
#define DOF_DILATE_RING_COUNT 3
#define DOF_FAST_GATHER_COC_ERROR 0.05

#define DOF_SHADER_DEFINES \
  "#define DOF_TILE_DIVISOR " STRINGIFY(DOF_TILE_DIVISOR) "\n" \
  "#define DOF_BOKEH_LUT_SIZE " STRINGIFY(DOF_BOKEH_LUT_SIZE) "\n" \
  "#define DOF_GATHER_RING_COUNT " STRINGIFY(DOF_GATHER_RING_COUNT) "\n" \
  "#define DOF_DILATE_RING_COUNT " STRINGIFY(DOF_DILATE_RING_COUNT) "\n" \
  "#define DOF_FAST_GATHER_COC_ERROR " STRINGIFY(DOF_FAST_GATHER_COC_ERROR) "\n"

struct PhoenixFrameBufferList : public wabi::HdBufferArray
{
  /**
   * ----------------------------------------------------------------------
   * PHOENIX. FRAMEBUFFER CREATION.
   * ----------------------------------------------------------------------
   * @CTOR: To create & register the Phoenix Rendering Engine framebuffers,
   *  to be consumed by Hydra for multiprimitve, indexed drawing. */
  PHOENIX_API
  PhoenixFrameBufferList(wabi::HdResourceRegistry *resourceRegistry,
                         wabi::TfToken const &role,
                         int count,
                         unsigned int commandNumUints);

  /**
   * ----------------------------------------------------------------------
   * PHOENIX. FRAMEBUFFER DESTRUCTION.
   * ----------------------------------------------------------------------
   * @DTOR: To destroy & unregister the Phoenix Rendering engine framebuffers. */
  PHOENIX_API
  ~PhoenixFrameBufferList() override;

  /**
   * ----------------------------------------------------------------------
   * PHOENIX. FRAMEBUFFER COMPACTION.
   * ----------------------------------------------------------------------
   * @GARBAGECOLLECTION: Performs compaction if necessary and returns true
   * if it becomes empty. */
  PHOENIX_API
  bool GarbageCollect() override;

  /**
   * ----------------------------------------------------------------------
   * PHOENIX. FRAMEBUFFER CAPACITY.
   * ----------------------------------------------------------------------
   * @CAPACITY: Returns the maximum number of FrameBuffer elements. */
  PHOENIX_API
  virtual size_t GetMaxNumElements() const;

  /**
   * ----------------------------------------------------------------------
   * PHOENIX. FRAMEBUFFER REALLOCATION.
   * ----------------------------------------------------------------------
   * @REALLOCATION: Performs reallocation. After reallocation, the buffer
   * will contain the specified @a ranges. If these ranges are currently
   * held by a different buffer array instance, then their data will be
   * copied from the specified @a curRangeOwner. */
  PHOENIX_API
  void Reallocate(std::vector<wabi::HdBufferArrayRangeSharedPtr> const &ranges,
                  wabi::HdBufferArraySharedPtr const &curRangeOwner) override;

  /**
   * ----------------------------------------------------------------------
   * PHOENIX. FRAMEBUFFER DEBUGGING.
   * ----------------------------------------------------------------------
   * @DEBUG: Log debug output, to troubleshoot in introspect the Phoenix
   * Rendering Engine FrameBuffer List. */
  PHOENIX_API
  void DebugDump(std::ostream &out) const override;

  /**
   * ----------------------------------------------------------------------
   * PHOENIX. THE RENDERING ENGINE FRAMEBUFFER.
   * ----------------------------------------------------------------------
   * @FRAMEBUFFER: The complete framebuffer list to be consumed by Hydra
   * for multiprimitve, indexed drawing. */

  /* Effects */
  struct GPUFrameBuffer *gtao_fb;
  struct GPUFrameBuffer *gtao_debug_fb;
  struct GPUFrameBuffer *downsample_fb;
  struct GPUFrameBuffer *maxzbuffer_fb;
  struct GPUFrameBuffer *bloom_blit_fb;
  struct GPUFrameBuffer *bloom_down_fb[MAX_BLOOM_STEP];
  struct GPUFrameBuffer *bloom_accum_fb[MAX_BLOOM_STEP - 1];
  struct GPUFrameBuffer *bloom_pass_accum_fb;
  struct GPUFrameBuffer *cryptomatte_fb;
  struct GPUFrameBuffer *shadow_accum_fb;
  struct GPUFrameBuffer *ssr_accum_fb;
  struct GPUFrameBuffer *sss_blur_fb;
  struct GPUFrameBuffer *sss_blit_fb;
  struct GPUFrameBuffer *sss_resolve_fb;
  struct GPUFrameBuffer *sss_clear_fb;
  struct GPUFrameBuffer *sss_translucency_fb;
  struct GPUFrameBuffer *sss_accum_fb;
  struct GPUFrameBuffer *dof_setup_fb;
  struct GPUFrameBuffer *dof_flatten_tiles_fb;
  struct GPUFrameBuffer *dof_dilate_tiles_fb;
  struct GPUFrameBuffer *dof_downsample_fb;
  struct GPUFrameBuffer *dof_reduce_fb;
  struct GPUFrameBuffer *dof_reduce_copy_fb;
  struct GPUFrameBuffer *dof_bokeh_fb;
  struct GPUFrameBuffer *dof_gather_fg_fb;
  struct GPUFrameBuffer *dof_filter_fg_fb;
  struct GPUFrameBuffer *dof_gather_fg_holefill_fb;
  struct GPUFrameBuffer *dof_gather_bg_fb;
  struct GPUFrameBuffer *dof_filter_bg_fb;
  struct GPUFrameBuffer *dof_scatter_fg_fb;
  struct GPUFrameBuffer *dof_scatter_bg_fb;
  struct GPUFrameBuffer *volumetric_fb;
  struct GPUFrameBuffer *volumetric_scat_fb;
  struct GPUFrameBuffer *volumetric_integ_fb;
  struct GPUFrameBuffer *volumetric_accum_fb;
  struct GPUFrameBuffer *screen_tracing_fb;
  struct GPUFrameBuffer *mist_accum_fb;
  struct GPUFrameBuffer *material_accum_fb;
  struct GPUFrameBuffer *renderpass_fb;
  struct GPUFrameBuffer *ao_accum_fb;
  struct GPUFrameBuffer *velocity_resolve_fb;
  struct GPUFrameBuffer *velocity_fb;
  struct GPUFrameBuffer *velocity_tiles_fb[2];

  struct GPUFrameBuffer *update_noise_fb;

  struct GPUFrameBuffer *planarref_fb;
  struct GPUFrameBuffer *planar_downsample_fb;

  struct GPUFrameBuffer *main_fb;
  struct GPUFrameBuffer *main_color_fb;
  struct GPUFrameBuffer *effect_fb;
  struct GPUFrameBuffer *effect_color_fb;
  struct GPUFrameBuffer *radiance_filtered_fb;
  struct GPUFrameBuffer *double_buffer_fb;
  struct GPUFrameBuffer *double_buffer_color_fb;
  struct GPUFrameBuffer *double_buffer_depth_fb;
  struct GPUFrameBuffer *taa_history_fb;
  struct GPUFrameBuffer *taa_history_color_fb;
};

typedef struct PhoenixGPUData
{
  void *engine_type;
  PhoenixFrameBufferList *fbl;
  char * /** TODO */ txl;
  char * /** TODO */ psl;
  char * /** TODO */ stl;
  void * /** TODO */ instance_data;

  char info[GPU_INFO_SIZE];
} PhoenixGPUData;

#ifdef __cplusplus
}
#endif
