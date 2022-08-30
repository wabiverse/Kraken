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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "KKE_main.h"

#include "ANCHOR_debug_codes.h"

#include "WM_debug_codes.h"

#include <wabi/base/tf/diagnostic.h>
#include <wabi/base/tf/envSetting.h>
#include <wabi/base/tf/setenv.h>

#include <wabi/base/plug/debugCodes.h>

#include <wabi/usd/sdf/debugCodes.h>
#include <wabi/usd/usd/debugCodes.h>

#if WITH_VULKAN
#  include <wabi/imaging/hgiVulkan/diagnostic.h>
#endif /* WITH_VULKAN */

KRAKEN_NAMESPACE_BEGIN

void KKE_kraken_enable_debug_codes()
{
  TfDebug::Enable(SDF_LAYER);
  TfDebug::Enable(SDF_CHANGES);
  TfDebug::Enable(SDF_ASSET);
  TfDebug::Enable(SDF_ASSET_TRACE_INVALID_CONTEXT);
  TfDebug::Enable(SDF_FILE_FORMAT);

  TfDebug::Enable(USD_AUTO_APPLY_API_SCHEMAS);
  TfDebug::Enable(USD_CHANGES);
  TfDebug::Enable(USD_CLIPS);
  TfDebug::Enable(USD_COMPOSITION);
  TfDebug::Enable(USD_DATA_BD);
  TfDebug::Enable(USD_DATA_BD_TRY);
  TfDebug::Enable(USD_INSTANCING);
  TfDebug::Enable(USD_PATH_RESOLUTION);
  TfDebug::Enable(USD_PAYLOADS);
  TfDebug::Enable(USD_PRIM_LIFETIMES);
  TfDebug::Enable(USD_SCHEMA_REGISTRATION);
  TfDebug::Enable(USD_STAGE_CACHE);
  TfDebug::Enable(USD_STAGE_LIFETIMES);
  TfDebug::Enable(USD_STAGE_OPEN);
  TfDebug::Enable(USD_STAGE_INSTANTIATION_TIME);
  TfDebug::Enable(USD_VALUE_RESOLUTION);
  TfDebug::Enable(USD_VALIDATE_VARIABILITY);

  /**
   * Debugging messages for Anchor. */
  // TfDebug::Enable(ANCHOR_SDL_VULKAN);
  // TfDebug::Enable(ANCHOR_DISPLAY_MANAGER);
  // TfDebug::Enable(ANCHOR_WIN32);

  /**
   * Debugging messages for MsgBus. */
  // TfDebug::Enable(KRAKEN_DEBUG_MSGBUS);
}


KRAKEN_NAMESPACE_END