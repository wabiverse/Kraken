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

#include "ANCHOR_api.h"

#include "ED_view3d.h"

#include "KKE_main.h"

#if WITH_VULKAN
#  include <vulkan/vulkan.h>
#endif /* WITH_VULKAN */

#include <wabi/base/arch/hints.h>

KRAKEN_NAMESPACE_BEGIN

void ED_view3d_run(bool *show)
{
  static AnchorWindowFlags flags;

  flags |= AnchorWindowFlags_MenuBar;

  if (!ANCHOR::Begin("View3D", show, flags)) {
    /** Optimization. Early out if Viewport isn't displayed. */
    ANCHOR::End();
    return;
  }

  static std::string graphics_api = "N/A";
  static std::string graphics_api_v = "N/A";

  static std::once_flag getVersion;
  std::call_once(getVersion, []() {
#if WITH_VULKAN
    graphics_api = "Vulkan";

    /** Only get the version once. */
    static auto version = PFN_vkEnumerateInstanceVersion(
      vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
    if (ARCH_UNLIKELY(version == nullptr)) {
      /** Backwards compatibility & no patch. */
      graphics_api_v = TfStringPrintf("%d.%d",
                                      VK_VERSION_MAJOR(VK_API_VERSION_1_0),
                                      VK_VERSION_MINOR(VK_API_VERSION_1_0));
    } else {
      /** Vulkan Version. */
      static uint32_t instance_version;
      version(&instance_version);
      graphics_api_v = TfStringPrintf("%d.%d.%d",
                                      VK_VERSION_MAJOR(instance_version),
                                      VK_VERSION_MINOR(instance_version),
                                      VK_VERSION_PATCH(instance_version));
    }
#endif /* WITH_VULKAN */
  });

  /** Display version in UI. */
  if (ANCHOR::BeginMenuBar()) {
    if (ANCHOR::BeginMenu(CHARALL(TfStringPrintf("Hydra on %s", CHARALL(graphics_api))))) {
      ANCHOR::MenuItem(
        CHARALL(TfStringPrintf("%s v", CHARALL(graphics_api), CHARALL(graphics_api_v))));
      ANCHOR::EndMenu();
    }
    ANCHOR::EndMenuBar();
  }

  ANCHOR::Text("I will be a viewport someday.");
  ANCHOR::End();
}

KRAKEN_NAMESPACE_END