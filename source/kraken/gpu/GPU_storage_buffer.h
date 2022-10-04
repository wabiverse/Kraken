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
 * Storage buffers API. Used to handle many way bigger buffers than Uniform buffers update at once.
 * Make sure that the data structure is compatible with what the implementation expect.
 * (see "7.8 Shader Buffer Variables and Shader Storage Blocks" from the OpenGL spec for more info
 * about std430 layout)
 * Rule of thumb: Padding to 16bytes, don't use vec3.
 */

#include "GPU_texture.h"
#include "GPU_vertex_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ListBase;

/** Opaque type hiding kraken::gpu::StorageBuf. */
typedef struct GPUStorageBuf GPUStorageBuf;

GPUStorageBuf *GPU_storagebuf_create_ex(size_t size,
                                        const void *data,
                                        GPUUsageType usage,
                                        const char *name);

#define GPU_storagebuf_create(size) \
  GPU_storagebuf_create_ex(size, NULL, GPU_USAGE_DYNAMIC, __func__);

void GPU_storagebuf_free(GPUStorageBuf *ssbo);

void GPU_storagebuf_update(GPUStorageBuf *ssbo, const void *data);

void GPU_storagebuf_bind(GPUStorageBuf *ssbo, int slot);
void GPU_storagebuf_unbind(GPUStorageBuf *ssbo);
void GPU_storagebuf_unbind_all(void);

void GPU_storagebuf_clear(GPUStorageBuf *ssbo,
                          eGPUTextureFormat internal_format,
                          eGPUDataFormat data_format,
                          void *data);
void GPU_storagebuf_clear_to_zero(GPUStorageBuf *ssbo);

/**
 * Read back content of the buffer to CPU for inspection.
 * Slow! Only use for inspection / debugging.
 * NOTE: Not synchronized. Use appropriate barrier before reading.
 */
void GPU_storagebuf_read(GPUStorageBuf *ssbo, void *data);

/**
 * \brief Copy a part of a vertex buffer to a storage buffer.
 *
 * @param ssbo: destination storage buffer
 * @param src: source vertex buffer
 * @param dst_offset: where to start copying to (in bytes).
 * @param src_offset: where to start copying from (in bytes).
 * @param copy_size: byte size of the segment to copy.
 */
void GPU_storagebuf_copy_sub_from_vertbuf(GPUStorageBuf *ssbo,
                                          GPUVertBuf *src,
                                          uint dst_offset,
                                          uint src_offset,
                                          uint copy_size);

#ifdef __cplusplus
}
#endif
