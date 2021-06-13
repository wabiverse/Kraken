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

#pragma once

#include "ANCHOR_api.h"
#include "ANCHOR_impl_vulkan.h"

#include <SDL_vulkan.h>

typedef SDL_Window ANCHOR_System;

/** todo: move to GPU */
typedef ANCHOR_ImplVulkanH_Window ANCHOR_SystemGPU;

typedef std::pair<SDL_Window *, ANCHOR_ImplVulkanH_Window *> HANDLE_sdl_vk_win;

HANDLE_sdl_vk_win ANCHOR_init_vulkan(VkResult &err);
eAnchorStatus ANCHOR_run_vulkan(ANCHOR_System *window, ANCHOR_SystemGPU *wd);
void ANCHOR_render_vulkan(ANCHOR_SystemGPU *wd);
void ANCHOR_clean_vulkan(ANCHOR_System *window /**Todo::VkResult &err*/);