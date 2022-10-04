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

#include "MEM_guardedalloc.h"
#include <cstring>

#include "KLI_kraklib.h"
#include "KLI_math_base.h"

#include "gpu_backend.hh"

#include "GPU_material.h"
#include "GPU_vertex_buffer.h" /* For GPUUsageType. */

#include "GPU_storage_buffer.h"
#include "gpu_storage_buffer_private.hh"
#include "gpu_vertex_buffer_private.hh"

/* -------------------------------------------------------------------- */
/** \name Creation & Deletion
 * \{ */

namespace kraken::gpu
{

  StorageBuf::StorageBuf(size_t size, const char *name)
  {
    /* Make sure that UBO is padded to size of vec4 */
    KLI_assert((size % 16) == 0);

    size_in_bytes_ = size;

    KLI_strncpy(name_, name, sizeof(name_));
  }

  StorageBuf::~StorageBuf()
  {
    MEM_SAFE_FREE(data_);
  }

}  // namespace kraken::gpu

/** \} */

/* -------------------------------------------------------------------- */
/** \name C-API
 * \{ */

using namespace kraken::gpu;

GPUStorageBuf *GPU_storagebuf_create_ex(size_t size,
                                        const void *data,
                                        GPUUsageType usage,
                                        const char *name)
{
  StorageBuf *ssbo = GPUBackend::get()->storagebuf_alloc(size, usage, name);
  /* Direct init. */
  if (data != nullptr) {
    ssbo->update(data);
  }
  return wrap(ssbo);
}

void GPU_storagebuf_free(GPUStorageBuf *ssbo)
{
  delete unwrap(ssbo);
}

void GPU_storagebuf_update(GPUStorageBuf *ssbo, const void *data)
{
  unwrap(ssbo)->update(data);
}

void GPU_storagebuf_bind(GPUStorageBuf *ssbo, int slot)
{
  unwrap(ssbo)->bind(slot);
}

void GPU_storagebuf_unbind(GPUStorageBuf *ssbo)
{
  unwrap(ssbo)->unbind();
}

void GPU_storagebuf_unbind_all()
{
  /* FIXME */
}

void GPU_storagebuf_clear(GPUStorageBuf *ssbo,
                          eGPUTextureFormat internal_format,
                          eGPUDataFormat data_format,
                          void *data)
{
  unwrap(ssbo)->clear(internal_format, data_format, data);
}

void GPU_storagebuf_clear_to_zero(GPUStorageBuf *ssbo)
{
  uint32_t data = 0u;
  GPU_storagebuf_clear(ssbo, GPU_R32UI, GPU_DATA_UINT, &data);
}

void GPU_storagebuf_copy_sub_from_vertbuf(GPUStorageBuf *ssbo,
                                          GPUVertBuf *src,
                                          uint dst_offset,
                                          uint src_offset,
                                          uint copy_size)
{
  unwrap(ssbo)->copy_sub(unwrap(src), dst_offset, src_offset, copy_size);
}

void GPU_storagebuf_read(GPUStorageBuf *ssbo, void *data)
{
  unwrap(ssbo)->read(data);
}

/** \} */
