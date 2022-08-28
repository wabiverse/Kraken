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

#if WITH_VULKAN
#  include <wabi/imaging/hgiVulkan/diagnostic.h>
#endif /* WITH_VULKAN */

KRAKEN_NAMESPACE_BEGIN

void KKE_kraken_enable_debug_codes()
{
  TfDebug::Enable(PLUG_LOAD);
  TfDebug::Enable(PLUG_REGISTRATION);
  TfDebug::Enable(PLUG_LOAD_IN_SECONDARY_THREAD);
  TfDebug::Enable(PLUG_INFO_SEARCH);

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