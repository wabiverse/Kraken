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
 */

#include "KKE_global.h"
#include "CLG_log.h"

namespace kraken
{
  namespace gpu
  {
    namespace debug
    {

      extern CLG_LogRef LOG;

      /* Initialize debugging. */
      void mtl_debug_init();

/* Using Macro's instead of variadic template due to non-string-literal
 * warning for CLG_logf when indirectly passing format string. */
#define EXPAND_ARGS(...) , ##__VA_ARGS__
#define MTL_LOG_ERROR(info, ...)               \
  {                                            \
    if (G.debug & G_DEBUG_GPU) {               \
      CLG_logf(debug::LOG.type,                \
               CLG_SEVERITY_ERROR,             \
               "[Metal Viewport Error]",       \
               "",                             \
               info EXPAND_ARGS(__VA_ARGS__)); \
    }                                          \
    KLI_assert(false);                         \
  }

#define MTL_LOG_WARNING(info, ...)             \
  {                                            \
    if (G.debug & G_DEBUG_GPU) {               \
      CLG_logf(debug::LOG.type,                \
               CLG_SEVERITY_WARN,              \
               "[Metal Viewport Warning]",     \
               "",                             \
               info EXPAND_ARGS(__VA_ARGS__)); \
    }                                          \
  }

#define MTL_LOG_INFO(info, ...)                \
  {                                            \
    if (G.debug & G_DEBUG_GPU) {               \
      CLG_logf(debug::LOG.type,                \
               CLG_SEVERITY_INFO,              \
               "[Metal Viewport Info]",        \
               "",                             \
               info EXPAND_ARGS(__VA_ARGS__)); \
    }                                          \
  }

    }  // namespace debug
  }    // namespace gpu
}  // namespace kraken
