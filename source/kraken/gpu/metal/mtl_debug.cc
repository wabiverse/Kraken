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

/**
 * @file
 * GPU.
 * Pixel Magic.
 */

#include "KLI_compiler_attrs.h"
#include "KLI_string.h"
#include "KLI_system.h"
#include "KLI_utildefines.h"

#include "KKE_global.h"

#include "GPU_debug.h"
#include "GPU_platform.h"

#include "mtl_context.hh"
#include "mtl_debug.hh"

#include "CLG_log.h"

#include <utility>

namespace kraken::gpu::debug
{

  CLG_LogRef LOG = {"gpu.debug.metal"};

  void mtl_debug_init()
  {
    CLOG_ENSURE(&LOG);
  }

}  // namespace kraken::gpu::debug

namespace kraken::gpu
{

  /* -------------------------------------------------------------------- */
  /** @name Debug Groups
   *
   * Useful for debugging through XCode GPU Debugger. This ensures all the API calls grouped into
   * "passes".
   * @{ */

  void MTLContext::debug_group_begin(const char *name, int index)
  {
    if (G.debug & G_DEBUG_GPU) {
      this->main_command_buffer.push_debug_group(name, index);
    }
  }

  void MTLContext::debug_group_end()
  {
    if (G.debug & G_DEBUG_GPU) {
      this->main_command_buffer.pop_debug_group();
    }
  }

  /** @} */

}  // namespace kraken::gpu
