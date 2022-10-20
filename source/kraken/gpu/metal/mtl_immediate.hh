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
 * Mimics old style opengl immediate mode drawing.
 */

#include "MEM_guardedalloc.h"
#include "gpu_immediate_private.hh"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

namespace kraken::gpu
{

  class MTLImmediate : public Immediate
  {
   private:

    MTLContext *m_context = nullptr;
    MTLTemporaryBuffer m_current_allocation;
    MTL::PrimitiveTopologyClass m_metal_primitive_mode;
    MTL::PrimitiveType m_metal_primitive_type;
    bool m_has_begun = false;

   public:

    MTLImmediate(MTLContext *ctx);
    ~MTLImmediate();

    uchar *begin() override;
    void end() override;
    bool imm_is_recording()
    {
      return m_has_begun;
    }
  };

}  // namespace kraken::gpu
