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

#pragma once

/**
 * @file
 * GPU.
 * Pixel Magic.
 */

#include "KLI_utildefines.h"

#include "GPU_vertex_format.h"

typedef enum
{
  /** Initial state. */
  GPU_VERTBUF_INVALID = 0,
  /** Was init with a vertex format. */
  GPU_VERTBUF_INIT = (1 << 0),
  /** Data has been touched and need to be re-uploaded. */
  GPU_VERTBUF_DATA_DIRTY = (1 << 1),
  /** The buffer has been created inside GPU memory. */
  GPU_VERTBUF_DATA_UPLOADED = (1 << 2),
} GPUVertBufStatus;

ENUM_OPERATORS(GPUVertBufStatus, GPU_VERTBUF_DATA_UPLOADED)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * How to create a #GPUVertBuf:
 * 1) verts = GPU_vertbuf_calloc()
 * 2) GPU_vertformat_attr_add(verts->format, ...)
 * 3) GPU_vertbuf_data_alloc(verts, vertex_len) <-- finalizes/packs vertex format
 * 4) GPU_vertbuf_attr_fill(verts, pos, application_pos_buffer)
 */

typedef enum
{
  /* can be extended to support more types */
  GPU_USAGE_STREAM = 0,
  GPU_USAGE_STATIC = 1, /* do not keep data in memory */
  GPU_USAGE_DYNAMIC = 2,
  GPU_USAGE_DEVICE_ONLY = 3, /* Do not do host->device data transfers. */

  /** Extended usage flags. */
  /* Flag for vertex buffers used for textures. Skips additional padding/compaction to ensure
   * format matches the texture exactly. Can be masked with other properties, and is stripped
   * during VertBuf::init. */
  GPU_USAGE_FLAG_BUFFER_TEXTURE_ONLY = 1 << 3,
} GPUUsageType;

ENUM_OPERATORS(GPUUsageType, GPU_USAGE_FLAG_BUFFER_TEXTURE_ONLY);

/** Opaque type hiding kraken::gpu::VertBuf. */
typedef struct GPUVertBuf GPUVertBuf;

/* Macros */
#define GPU_VERTBUF_DISCARD_SAFE(verts) \
  do {                                  \
    if (verts != NULL) {                \
      GPU_vertbuf_discard(verts);       \
      verts = NULL;                     \
    }                                   \
  } while (0)

#ifdef __cplusplus
}
#endif