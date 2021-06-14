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
 * @file
 * Anchor.
 * Bare Metal.
 */

#include "ANCHOR_api.h"
#include "ANCHOR_system.h"

/* todo: more backends */
#include "ANCHOR_vulkan.h"

#include <wabi/base/tf/error.h>

WABI_NAMESPACE_USING

ANCHOR_SystemHandle ANCHOR_CreateSystem(int backend)
{
  switch (backend) {
    case (ANCHOR_SDL | ANCHOR_VULKAN): {
      VkResult vk_err;
      return (ANCHOR_SystemHandle)ANCHOR_init_vulkan(vk_err);
    }

    default:
      TF_CODING_ERROR("Specified a backend which is not implemented.");
      exit(ANCHOR_ERROR);
  }
}

void ANCHOR_DestroySystem(ANCHOR_SystemHandle *system)
{
  ANCHOR_clean_vulkan(system);
}