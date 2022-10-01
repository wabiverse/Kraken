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
 * @file Draw.
 * Spontaneous Expression.
 *
 * List of defines that are shared with the GPUShaderCreateInfos. We do this to avoid
 * dragging larger headers into the createInfo pipeline which would cause problems.
 */

#ifndef GPU_SHADER
#  include "KLI_span.hh"
#  include "GPU_shader_shared_utils.h"

namespace kraken::draw::command {

struct RecordingState;

#endif

/* -------------------------------------------------------------------- */
/** \name Multi Draw
 * \{ */

/**
 * A DrawGroup allow to split the command stream into batch-able chunks of commands with
 * the same render state.
 */
struct DrawGroup {
  /** Index of next #DrawGroup from the same header. */
  uint next;

  /** Index of the first instances after sorting. */
  uint start;
  /** Total number of instances (including inverted facing). Needed to issue the draw call. */
  uint len;
  /** Number of non inverted scaling instances in this Group. */
  uint front_facing_len;

  /** #GPUBatch values to be copied to #DrawCommand after sorting (if not overridden). */
  int vertex_len;
  int vertex_first;
  int base_index;

  /** Atomic counters used during command sorting. */
  uint total_counter;

#ifndef GPU_SHADER
  /* NOTE: Union just to make sure the struct has always the same size on all platform. */
  union {
    struct {
      /** For debug printing only. */
      uint front_proto_len;
      uint back_proto_len;
      /** Needed to create the correct draw call. */
      GPUBatch *gpu_batch;
    };
    struct {
#endif
      uint front_facing_counter;
      uint back_facing_counter;
      uint _pad0, _pad1;
#ifndef GPU_SHADER
    };
  };
#endif
};
KLI_STATIC_ASSERT_ALIGN(DrawGroup, 16)

/**
 * Representation of a future draw call inside a DrawGroup. This #DrawPrototype is then
 * converted into #DrawCommand on GPU after visibility and compaction. Multiple
 * #DrawPrototype might get merged into the same final #DrawCommand.
 */
struct DrawPrototype {
  /* Reference to parent DrawGroup to get the GPUBatch vertex / instance count. */
  uint group_id;
  /* Resource handle associated with this call. Also reference visibility. */
  uint resource_handle;
  /* Number of instances. */
  uint instance_len;
  uint _pad0;
};
KLI_STATIC_ASSERT_ALIGN(DrawPrototype, 16)

/** \} */

#ifndef GPU_SHADER
};  // namespace kraken::draw::command
#endif
