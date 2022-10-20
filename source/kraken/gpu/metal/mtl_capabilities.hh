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
 * Derived from original work by Copyright 2022, Blender Foundation.
 * From the Blender GPU library. (source/blender/gpu).
 *
 * With any additions or modifications specific to Kraken.
 *
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

#pragma once

/**
 * @file
 * @ingroup GPU.
 * Pixel Magic.
 *
 * The Kraken GPU capabilities - for Apple Metal.
 */

namespace kraken
{
  namespace gpu
  {

    /*** Derived from: https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf ***/
    /** Upper Bound/Fixed Limits **/

#define MTL_MAX_TEXTURE_SLOTS 128
#define MTL_MAX_SAMPLER_SLOTS MTL_MAX_TEXTURE_SLOTS
/* Max limit without using bind-less for samplers. */
#define MTL_MAX_DEFAULT_SAMPLERS 16
#define MTL_MAX_UNIFORM_BUFFER_BINDINGS 31
#define MTL_MAX_VERTEX_INPUT_ATTRIBUTES 31
#define MTL_MAX_UNIFORMS_PER_BLOCK 64

    /* Context-specific limits -- populated in 'MTLBackend::platform_init' */
    struct MTLCapabilities
    {

      /* Variable Limits & feature sets. */
      int max_color_render_targets = 4;          /* Minimum = 4 */
      int buffer_alignment_for_textures = 256;   /* Upper bound = 256 bytes */
      int minimum_buffer_offset_alignment = 256; /* Upper bound = 256 bytes */

      /* Capabilities */
      bool supports_vertex_amplification = false;
      bool supports_texture_swizzle = true;
      bool supports_cubemaps = true;
      bool supports_layered_rendering = true;
      bool supports_memory_barriers = false;
      bool supports_sampler_border_color = false;
      bool supports_argument_buffers_tier2 = false;

      /* GPU Family */
      bool supports_family_mac1 = false;
      bool supports_family_mac2 = false;
      bool supports_family_mac_catalyst1 = false;
      bool supports_family_mac_catalyst2 = false;
    };

  }  // namespace gpu
}  // namespace kraken
