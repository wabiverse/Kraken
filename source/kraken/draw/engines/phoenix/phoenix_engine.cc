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
 * @file Draw.
 * Spontaneous Expression.
 *
 * The Phoenix Render Engine.
 * The OpenSubdiv-based real-time render engine of the 21st century.
 */

#include <memory>
#include <mutex>

#include "DRW_render.h"

#include "USD_world.h"

#include "IMB_imbuf.h"

#include "phoenix_private.h"

#include "phoenix_engine.h" /* own include */

#define PHOENIX_ENGINE "KRAKEN_PHOENIX"

using namespace wabi;



static const DrawEngineDataSize phoenix_data_size = DRW_VIEWPORT_DATA_SIZE(PhoenixGPUData);

DrawEngineType draw_engine_phoenix_type = {
  NULL,
  NULL,
  N_("Phoenix"),
  &phoenix_data_size,
  NULL,
  NULL,
  NULL, /* instance_free */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

RenderEngineType DRW_engine_viewport_phoenix_type = {
  NULL,
  NULL,
  PHOENIX_ENGINE,
  N_("Phoenix"),
  RE_INTERNAL | RE_USE_PREVIEW | RE_USE_STEREO_VIEWPORT | RE_USE_GPU_CONTEXT,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &draw_engine_phoenix_type,
  {NULL, NULL, NULL},
};

#undef PHOENIX_ENGINE
