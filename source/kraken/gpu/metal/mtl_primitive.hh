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
 * GPU.
 * Pixel Magic.
 *
 * Encapsulation of Frame-buffer states (attached textures, viewport, scissors).
 */

#include "KLI_assert.h"

#include "GPU_primitive.h"

#include <Metal/Metal.hpp>

namespace kraken::gpu
{

  /** Utility functions **/
  static inline MTL::PrimitiveTopologyClass mtl_prim_type_to_topology_class(
    MTL::PrimitiveType prim_type)
  {
    switch (prim_type) {
      case MTL::PrimitiveTypePoint:
        return MTL::PrimitiveTopologyClassPoint;
      case MTL::PrimitiveTypeLine:
      case MTL::PrimitiveTypeLineStrip:
        return MTL::PrimitiveTopologyClassLine;
      case MTL::PrimitiveTypeTriangle:
      case MTL::PrimitiveTypeTriangleStrip:
        return MTL::PrimitiveTopologyClassTriangle;
    }
    return MTL::PrimitiveTopologyClassUnspecified;
  }

  static inline MTL::PrimitiveType gpu_prim_type_to_metal(GPUPrimType prim_type)
  {
    switch (prim_type) {
      case GPU_PRIM_POINTS:
        return MTL::PrimitiveTypePoint;
      case GPU_PRIM_LINES:
      case GPU_PRIM_LINES_ADJ:
      case GPU_PRIM_LINE_LOOP:
        return MTL::PrimitiveTypeLine;
      case GPU_PRIM_LINE_STRIP:
      case GPU_PRIM_LINE_STRIP_ADJ:
        return MTL::PrimitiveTypeLineStrip;
      case GPU_PRIM_TRIS:
      case GPU_PRIM_TRI_FAN:
      case GPU_PRIM_TRIS_ADJ:
        return MTL::PrimitiveTypeTriangle;
      case GPU_PRIM_TRI_STRIP:
        return MTL::PrimitiveTypeTriangleStrip;
      case GPU_PRIM_NONE:
        return MTL::PrimitiveTypePoint;
    };
  }

  /* Certain primitive types are not supported in Metal, and require emulation.
   * `GPU_PRIM_LINE_LOOP` and  `GPU_PRIM_TRI_FAN` required index buffer patching.
   * Adjacency types do not need emulation as the input structure is the same,
   * and access is controlled from the vertex shader through SSBO vertex fetch.
   * -- These Adj cases are only used in geometry shaders in OpenGL. */
  static inline bool mtl_needs_topology_emulation(GPUPrimType prim_type)
  {

    KLI_assert(prim_type != GPU_PRIM_NONE);
    switch (prim_type) {
      case GPU_PRIM_LINE_LOOP:
      case GPU_PRIM_TRI_FAN:
        return true;
      default:
        return false;
    }
    return false;
  }

  static inline bool mtl_vertex_count_fits_primitive_type(uint32_t vertex_count,
                                                          MTL::PrimitiveType prim_type)
  {
    if (vertex_count == 0) {
      return false;
    }

    switch (prim_type) {
      case MTL::PrimitiveTypeLineStrip:
        return (vertex_count > 1);
      case MTL::PrimitiveTypeLine:
        return (vertex_count % 2 == 0);
      case MTL::PrimitiveTypePoint:
        return (vertex_count > 0);
      case MTL::PrimitiveTypeTriangle:
        return (vertex_count % 3 == 0);
      case MTL::PrimitiveTypeTriangleStrip:
        return (vertex_count > 2);
    }
    KLI_assert(false);
    return false;
  }

}  // namespace kraken::gpu
