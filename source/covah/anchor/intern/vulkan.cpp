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
#include "ANCHOR_debug_codes.h"
#include "ANCHOR_impl_sdl.h"
#include "ANCHOR_impl_vulkan.h"
#include "ANCHOR_vulkan.h"

#include "CKE_version.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_vulkan.h>
#include <iostream> /* cout */
#include <mutex>    /* once */
#include <stdio.h>  /* printf, fprintf */
#include <stdlib.h> /* abort */
#include <vulkan/vulkan.h>

#include <wabi/base/arch/systemInfo.h>
#include <wabi/base/tf/error.h>
#include <wabi/base/tf/stringUtils.h>
#include <wabi/base/tf/warning.h>

#include <wabi/imaging/hd/driver.h>
#include <wabi/imaging/hgiVulkan/diagnostic.h>
#include <wabi/imaging/hgiVulkan/hgi.h>
#include <wabi/imaging/hgiVulkan/instance.h>

#include <wabi/usdImaging/usdApollo/engine.h>
#include <wabi/usdImaging/usdApollo/renderParams.h>

WABI_NAMESPACE_USING
