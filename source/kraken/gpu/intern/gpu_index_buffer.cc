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
 *
 * GPU element list (AKA index buffer).
 */

#include "MEM_guardedalloc.h"

#include "KLI_math_base.h"
#include "KLI_utildefines.h"

#include "gpu_backend.hh"

#include "gpu_index_buffer_private.hh"

#include "GPU_platform.h"

#include <cstring>

#define KEEP_SINGLE_COPY 1

#define RESTART_INDEX 0xFFFFFFFF

/* -------------------------------------------------------------------- */
/** \name IndexBufBuilder
 * \{ */

using namespace kraken;
using namespace kraken::gpu;

void GPU_indexbuf_init_ex(GPUIndexBufBuilder *builder,
                          GPUPrimType prim_type,
                          uint index_len,
                          uint vertex_len)
{
  builder->max_allowed_index = vertex_len - 1;
  builder->max_index_len = index_len;
  builder->index_len = 0;  // start empty
  builder->index_min = UINT32_MAX;
  builder->index_max = 0;
  builder->prim_type = prim_type;

#ifdef __APPLE__
  /* Only encode restart indices for restart-compatible primitive types.
   * Resolves out-of-bounds read error on macOS. Using 0-index will ensure
   * degenerative primitives when skipping primitives is required and will
   * incur no additional performance cost for rendering. */
  if (GPU_type_matches_ex(GPU_DEVICE_ANY, GPU_OS_MAC, GPU_DRIVER_ANY, GPU_BACKEND_METAL)) {
    /* We will still use restart-indices for point primitives and then
     * patch these during IndexBuf::init, as we cannot benefit from degenerative
     * primitives to eliminate these. */
    builder->restart_index_value = (is_restart_compatible(prim_type) ||
                                    prim_type == GPU_PRIM_POINTS) ?
                                     RESTART_INDEX :
                                     0;
  } else {
    builder->restart_index_value = RESTART_INDEX;
  }
#else
  builder->restart_index_value = RESTART_INDEX;
#endif
  builder->uses_restart_indices = false;
  builder->data = (uint *)MEM_callocN(builder->max_index_len * sizeof(uint), "GPUIndexBuf data");
}

void GPU_indexbuf_init(GPUIndexBufBuilder *builder,
                       GPUPrimType prim_type,
                       uint prim_len,
                       uint vertex_len)
{
  int verts_per_prim = GPU_indexbuf_primitive_len(prim_type);
#if TRUST_NO_ONE
  assert(verts_per_prim != -1);
#endif
  GPU_indexbuf_init_ex(builder, prim_type, prim_len * uint(verts_per_prim), vertex_len);
}

GPUIndexBuf *GPU_indexbuf_build_on_device(uint index_len)
{
  GPUIndexBuf *elem_ = GPU_indexbuf_calloc();
  GPU_indexbuf_init_build_on_device(elem_, index_len);
  return elem_;
}

void GPU_indexbuf_init_build_on_device(GPUIndexBuf *elem, uint index_len)
{
  IndexBuf *elem_ = unwrap(elem);
  elem_->init_build_on_device(index_len);
}

void GPU_indexbuf_join(GPUIndexBufBuilder *builder_to, const GPUIndexBufBuilder *builder_from)
{
  KLI_assert(builder_to->data == builder_from->data);
  builder_to->index_len = max_uu(builder_to->index_len, builder_from->index_len);
  builder_to->index_min = min_uu(builder_to->index_min, builder_from->index_min);
  builder_to->index_max = max_uu(builder_to->index_max, builder_from->index_max);
}

void GPU_indexbuf_add_generic_vert(GPUIndexBufBuilder *builder, uint v)
{
#if TRUST_NO_ONE
  assert(builder->data != nullptr);
  assert(builder->index_len < builder->max_index_len);
  assert(v <= builder->max_allowed_index);
#endif
  builder->data[builder->index_len++] = v;
  builder->index_min = MIN2(builder->index_min, v);
  builder->index_max = MAX2(builder->index_max, v);
}

void GPU_indexbuf_add_primitive_restart(GPUIndexBufBuilder *builder)
{
#if TRUST_NO_ONE
  assert(builder->data != nullptr);
  assert(builder->index_len < builder->max_index_len);
#endif
  builder->data[builder->index_len++] = builder->restart_index_value;
  builder->uses_restart_indices = true;
}

void GPU_indexbuf_add_point_vert(GPUIndexBufBuilder *builder, uint v)
{
#if TRUST_NO_ONE
  assert(builder->prim_type == GPU_PRIM_POINTS);
#endif
  GPU_indexbuf_add_generic_vert(builder, v);
}

void GPU_indexbuf_add_line_verts(GPUIndexBufBuilder *builder, uint v1, uint v2)
{
#if TRUST_NO_ONE
  assert(builder->prim_type == GPU_PRIM_LINES);
  assert(v1 != v2);
#endif
  GPU_indexbuf_add_generic_vert(builder, v1);
  GPU_indexbuf_add_generic_vert(builder, v2);
}

void GPU_indexbuf_add_tri_verts(GPUIndexBufBuilder *builder, uint v1, uint v2, uint v3)
{
#if TRUST_NO_ONE
  assert(builder->prim_type == GPU_PRIM_TRIS);
  assert(v1 != v2 && v2 != v3 && v3 != v1);
#endif
  GPU_indexbuf_add_generic_vert(builder, v1);
  GPU_indexbuf_add_generic_vert(builder, v2);
  GPU_indexbuf_add_generic_vert(builder, v3);
}

void GPU_indexbuf_add_line_adj_verts(GPUIndexBufBuilder *builder,
                                     uint v1,
                                     uint v2,
                                     uint v3,
                                     uint v4)
{
#if TRUST_NO_ONE
  assert(builder->prim_type == GPU_PRIM_LINES_ADJ);
  assert(v2 != v3); /* only the line need diff indices */
#endif
  GPU_indexbuf_add_generic_vert(builder, v1);
  GPU_indexbuf_add_generic_vert(builder, v2);
  GPU_indexbuf_add_generic_vert(builder, v3);
  GPU_indexbuf_add_generic_vert(builder, v4);
}

void GPU_indexbuf_set_point_vert(GPUIndexBufBuilder *builder, uint elem, uint v1)
{
  KLI_assert(builder->prim_type == GPU_PRIM_POINTS);
  KLI_assert(elem < builder->max_index_len);
  builder->data[elem++] = v1;
  builder->index_min = MIN2(builder->index_min, v1);
  builder->index_max = MAX2(builder->index_max, v1);
  builder->index_len = MAX2(builder->index_len, elem);
}

void GPU_indexbuf_set_line_verts(GPUIndexBufBuilder *builder, uint elem, uint v1, uint v2)
{
  KLI_assert(builder->prim_type == GPU_PRIM_LINES);
  KLI_assert(v1 != v2);
  KLI_assert(v1 <= builder->max_allowed_index);
  KLI_assert(v2 <= builder->max_allowed_index);
  KLI_assert((elem + 1) * 2 <= builder->max_index_len);
  uint idx = elem * 2;
  builder->data[idx++] = v1;
  builder->data[idx++] = v2;
  builder->index_min = MIN3(builder->index_min, v1, v2);
  builder->index_max = MAX3(builder->index_max, v1, v2);
  builder->index_len = MAX2(builder->index_len, idx);
}

void GPU_indexbuf_set_tri_verts(GPUIndexBufBuilder *builder, uint elem, uint v1, uint v2, uint v3)
{
  KLI_assert(builder->prim_type == GPU_PRIM_TRIS);
  KLI_assert(v1 != v2 && v2 != v3 && v3 != v1);
  KLI_assert(v1 <= builder->max_allowed_index);
  KLI_assert(v2 <= builder->max_allowed_index);
  KLI_assert(v3 <= builder->max_allowed_index);
  KLI_assert((elem + 1) * 3 <= builder->max_index_len);
  uint idx = elem * 3;
  builder->data[idx++] = v1;
  builder->data[idx++] = v2;
  builder->data[idx++] = v3;

  builder->index_min = MIN4(builder->index_min, v1, v2, v3);
  builder->index_max = MAX4(builder->index_max, v1, v2, v3);
  builder->index_len = MAX2(builder->index_len, idx);
}

void GPU_indexbuf_set_point_restart(GPUIndexBufBuilder *builder, uint elem)
{
  KLI_assert(builder->prim_type == GPU_PRIM_POINTS);
  KLI_assert(elem < builder->max_index_len);
  builder->data[elem++] = builder->restart_index_value;
  builder->index_len = MAX2(builder->index_len, elem);
  builder->uses_restart_indices = true;
}

void GPU_indexbuf_set_line_restart(GPUIndexBufBuilder *builder, uint elem)
{
  KLI_assert(builder->prim_type == GPU_PRIM_LINES);
  KLI_assert((elem + 1) * 2 <= builder->max_index_len);
  uint idx = elem * 2;
  builder->data[idx++] = builder->restart_index_value;
  builder->data[idx++] = builder->restart_index_value;
  builder->index_len = MAX2(builder->index_len, idx);
  builder->uses_restart_indices = true;
}

void GPU_indexbuf_set_tri_restart(GPUIndexBufBuilder *builder, uint elem)
{
  KLI_assert(builder->prim_type == GPU_PRIM_TRIS);
  KLI_assert((elem + 1) * 3 <= builder->max_index_len);
  uint idx = elem * 3;
  builder->data[idx++] = builder->restart_index_value;
  builder->data[idx++] = builder->restart_index_value;
  builder->data[idx++] = builder->restart_index_value;
  builder->index_len = MAX2(builder->index_len, idx);
  builder->uses_restart_indices = true;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Creation & Deletion
 * \{ */

namespace kraken::gpu
{

  IndexBuf::~IndexBuf()
  {
    if (!is_subrange_) {
      MEM_SAFE_FREE(data_);
    }
  }

  void IndexBuf::init(uint indices_len,
                      uint32_t *indices,
                      uint min_index,
                      uint max_index,
                      GPUPrimType prim_type,
                      bool uses_restart_indices)
  {
    is_init_ = true;
    data_ = indices;
    index_start_ = 0;
    index_len_ = indices_len;
    is_empty_ = min_index > max_index;

    /* Patch index buffer to remove restart indices from
     * non-restart-compatible primitive types. Restart indices
     * are situationally added to selectively hide vertices.
     * Metal does not support restart-indices for non-restart-compatible
     * types, as such we should remove these indices.
     *
     * We only need to perform this for point primitives, as
     * line primitives/triangle primitives can use index 0 for all
     * vertices to create a degenerative primitive, where all
     * vertices share the same index and skip rendering via HW
     * culling. */
    if (prim_type == GPU_PRIM_POINTS && uses_restart_indices) {
      this->strip_restart_indices();
    }

#if GPU_TRACK_INDEX_RANGE
    /* Everything remains 32 bit while building to keep things simple.
     * Find min/max after, then convert to smallest index type possible. */
    uint range = min_index < max_index ? max_index - min_index : 0;
    /* count the primitive restart index. */
    range += 1;

    if (range <= 0xFFFF) {
      index_type_ = GPU_INDEX_U16;
      bool do_clamp_indices = false;
#  ifdef __APPLE__
      /* NOTE: For the Metal Backend, we use degenerative primitives to hide vertices
       * which are not restart compatible. When this is done, we need to ensure
       * that compressed index ranges clamp all index values within the valid
       * range, rather than maximally clamping against the USHORT restart index
       * value of 0xFFFFu, as this will cause an out-of-bounds read during
       * vertex assembly. */
      do_clamp_indices = GPU_type_matches_ex(GPU_DEVICE_ANY,
                                             GPU_OS_MAC,
                                             GPU_DRIVER_ANY,
                                             GPU_BACKEND_METAL);
#  endif
      this->squeeze_indices_short(min_index, max_index, prim_type, do_clamp_indices);
    }
#endif
  }

  void IndexBuf::init_build_on_device(uint index_len)
  {
    is_init_ = true;
    index_start_ = 0;
    index_len_ = index_len;
    index_type_ = GPU_INDEX_U32;
    data_ = nullptr;
  }

  void IndexBuf::init_subrange(IndexBuf *elem_src, uint start, uint length)
  {
    /* We don't support nested subranges. */
    KLI_assert(elem_src && elem_src->is_subrange_ == false);
    KLI_assert((length == 0) || (start + length <= elem_src->index_len_));

    is_init_ = true;
    is_subrange_ = true;
    src_ = elem_src;
    index_start_ = start;
    index_len_ = length;
    index_base_ = elem_src->index_base_;
    index_type_ = elem_src->index_type_;
  }

  uint IndexBuf::index_range(uint *r_min, uint *r_max)
  {
    if (index_len_ == 0) {
      *r_min = *r_max = 0;
      return 0;
    }
    const uint32_t *uint_idx = (uint32_t *)data_;
    uint min_value = RESTART_INDEX;
    uint max_value = 0;
    for (uint i = 0; i < index_len_; i++) {
      const uint value = uint_idx[i];
      if (value == RESTART_INDEX) {
        continue;
      }
      if (value < min_value) {
        min_value = value;
      } else if (value > max_value) {
        max_value = value;
      }
    }
    if (min_value == RESTART_INDEX) {
      *r_min = *r_max = 0;
      return 0;
    }
    *r_min = min_value;
    *r_max = max_value;
    return max_value - min_value;
  }

  void IndexBuf::squeeze_indices_short(uint min_idx,
                                       uint max_idx,
                                       GPUPrimType prim_type,
                                       bool clamp_indices_in_range)
  {
    /* data will never be *larger* than builder->data...
     * converting in place to avoid extra allocation */
    uint16_t *ushort_idx = (uint16_t *)data_;
    const uint32_t *uint_idx = (uint32_t *)data_;

    if (max_idx >= 0xFFFF) {
      index_base_ = min_idx;
      /* NOTE: When using restart_index=0 for degenerative primitives indices,
       * the compressed index will go below zero and wrap around when min_idx > 0.
       * In order to ensure the resulting index is still within range, we instead
       * clamp index to the maximum within the index range.
       *
       * `clamp_max_idx` represents the maximum possible index to clamp against. If primitive is
       * restart-compatible, we can just clamp against the primitive-restart value, otherwise, we
       * must assign to a valid index within the range.
       *
       * NOTE: For OpenGL we skip this by disabling clamping, as we still need to use
       * restart index values for point primitives to disable rendering. */
      uint16_t clamp_max_idx = (is_restart_compatible(prim_type) || !clamp_indices_in_range) ?
                                 0xFFFFu :
                                 (max_idx - min_idx);
      for (uint i = 0; i < index_len_; i++) {
        ushort_idx[i] = uint16_t(MIN2(clamp_max_idx, uint_idx[i] - min_idx));
      }
    } else {
      index_base_ = 0;
      for (uint i = 0; i < index_len_; i++) {
        ushort_idx[i] = uint16_t(uint_idx[i]);
      }
    }
  }

  uint32_t *IndexBuf::unmap(const uint32_t *mapped_memory) const
  {
    size_t size = size_get();
    uint32_t *result = static_cast<uint32_t *>(MEM_mallocN(size, __func__));
    memcpy(result, mapped_memory, size);
    return result;
  }

}  // namespace kraken::gpu

/** \} */

/* -------------------------------------------------------------------- */
/** \name C-API
 * \{ */

GPUIndexBuf *GPU_indexbuf_calloc()
{
  return wrap(GPUBackend::get()->indexbuf_alloc());
}

GPUIndexBuf *GPU_indexbuf_build(GPUIndexBufBuilder *builder)
{
  GPUIndexBuf *elem = GPU_indexbuf_calloc();
  GPU_indexbuf_build_in_place(builder, elem);
  return elem;
}

GPUIndexBuf *GPU_indexbuf_create_subrange(GPUIndexBuf *elem_src, uint start, uint length)
{
  GPUIndexBuf *elem = GPU_indexbuf_calloc();
  GPU_indexbuf_create_subrange_in_place(elem, elem_src, start, length);
  return elem;
}

void GPU_indexbuf_build_in_place(GPUIndexBufBuilder *builder, GPUIndexBuf *elem)
{
  KLI_assert(builder->data != nullptr);
  /* Transfer data ownership to GPUIndexBuf.
   * It will be uploaded upon first use. */
  unwrap(elem)->init(builder->index_len,
                     builder->data,
                     builder->index_min,
                     builder->index_max,
                     builder->prim_type,
                     builder->uses_restart_indices);
  builder->data = nullptr;
}

void GPU_indexbuf_create_subrange_in_place(GPUIndexBuf *elem,
                                           GPUIndexBuf *elem_src,
                                           uint start,
                                           uint length)
{
  unwrap(elem)->init_subrange(unwrap(elem_src), start, length);
}

const uint32_t *GPU_indexbuf_read(GPUIndexBuf *elem)
{
  return unwrap(elem)->read();
}

uint32_t *GPU_indexbuf_unmap(const GPUIndexBuf *elem, const uint32_t *mapped_buffer)
{
  return unwrap(elem)->unmap(mapped_buffer);
}

void GPU_indexbuf_discard(GPUIndexBuf *elem)
{
  delete unwrap(elem);
}

bool GPU_indexbuf_is_init(GPUIndexBuf *elem)
{
  return unwrap(elem)->is_init();
}

int GPU_indexbuf_primitive_len(GPUPrimType prim_type)
{
  return indices_per_primitive(prim_type);
}

void GPU_indexbuf_use(GPUIndexBuf *elem)
{
  unwrap(elem)->upload_data();
}

void GPU_indexbuf_bind_as_ssbo(GPUIndexBuf *elem, int binding)
{
  unwrap(elem)->bind_as_ssbo(binding);
}

void GPU_indexbuf_update_sub(GPUIndexBuf *elem, uint start, uint len, const void *data)
{
  unwrap(elem)->update_sub(start, len, data);
}

/** \} */