/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2016 Blender Foundation. */

/** \file
 * \ingroup draw
 */

#include <stdio.h>

#include "KLI_listbase.h"
#include "KLI_rect.h"
#include "KLI_string.h"
#include "KLI_task.h"
#include "KLI_threads.h"

#include "KRF_api.h"

#include "KKE_colortools.h"
#include "KKE_context.h"
#include "KKE_global.h"
#include "KKE_main.h"

#include "USD_userdef_types.h"

// #include "ED_gpencil.h"
#include "ED_screen.h"
#include "ED_space_api.h"
// #include "ED_view3d.h"

#include "GPU_capabilities.h"
#include "GPU_framebuffer.h"
#include "GPU_immediate.h"
#include "GPU_matrix.h"
#include "GPU_platform.h"
#include "GPU_shader_shared.h"
#include "GPU_state.h"
#include "GPU_uniform_buffer.h"
#include "GPU_viewport.h"

#include "IMB_colormanagement.h"

#include "RE_engine.h"
// #include "RE_pipeline.h"

#include "UI_resources.h"
#include "UI_view2d.h"

#include "WM_window.h"

// #include "draw_color_management.h"
#include "draw_manager.h"
// #include "draw_manager_profiling.h"
// #include "draw_manager_testing.h"
// #include "draw_manager_text.h"
// #include "draw_shader.h"
// #include "draw_subdivision.h"
// #include "draw_texture_pool.h"

/* only for callbacks */
// #include "draw_cache_impl.h"

// #include "engines/basic/basic_engine.h"
// #include "engines/compositor/compositor_engine.h"
// #include "engines/phoenix/phoenix_engine.h"
// #include "engines/phoenix_next/phoenix_engine.h"
// #include "engines/external/external_engine.h"
// #include "engines/gpencil/gpencil_engine.h"
// #include "engines/image/image_engine.h"
// #include "engines/overlay/overlay_engine.h"
// #include "engines/select/select_engine.h"
// #include "engines/workbench/workbench_engine.h"

#include "GPU_context.h"

#include "DRW_engine.h"
// #include "DRW_select_buffer.h"

/** Render State: No persistent data between draw calls. */
DRWManager DST = {NULL};

void DRW_gpu_context_create(void)
{
  KLI_assert(DST.gl_context == NULL); /* Ensure it's called once */

  DST.gl_context_mutex = KLI_ticket_mutex_alloc();
  /* This changes the active context. */
  DST.gl_context = WM_gpu_context_create();
  WM_gpu_context_activate(DST.gl_context);
  /* Be sure to create gpu_context too. */
  DST.gpu_context = GPU_context_create(0, DST.gl_context);
  /* So we activate the window's one afterwards. */
  WM_window_reset_drawable();
}

void DRW_engines_register(void)
{
  RE_engines_register(&DRW_engine_viewport_phoenix_type);
}