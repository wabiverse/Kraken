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

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "KKE_global.h"

#include "USD_userdef_types.h"

#include "mtl_context.hh"
#include "mtl_debug.hh"
#include "mtl_memory.hh"

using namespace kraken;
using namespace kraken::gpu;

namespace kraken::gpu
{

  /* -------------------------------------------------------------------- */
  /** @name Memory Management - MTLBufferPool and MTLSafeFreeList implementations. */

  void MTLBufferPool::init(MTL::Device *mtl_device)
  {
    if (!m_ensure_initialised) {
      KLI_assert(mtl_device);
      m_ensure_initialised = true;
      m_device = mtl_device;

#if MTL_DEBUG_MEMORY_STATISTICS == 1
      /* Debug statistics. */
      m_per_frame_allocation_count = 0;
      m_allocations_in_pool = 0;
      m_buffers_in_pool = 0;
#endif

      /* Free pools -- Create initial safe free pool */
      KLI_assert(m_current_free_list == nullptr);
      this->begin_new_safe_list();
    }
  }

  MTLBufferPool::~MTLBufferPool()
  {
    this->free();
  }

  void MTLBufferPool::free()
  {

    for (auto buffer : m_allocations) {
      KLI_assert(buffer);
      delete buffer;
    }
    m_allocations.clear();

    for (std::multiset<kraken::gpu::MTLBufferHandle, kraken::gpu::CompareMTLBuffer> *buffer_pool :
         m_buffer_pools.values()) {
      delete buffer_pool;
    }
    m_buffer_pools.clear();
  }

  gpu::MTLBuffer *MTLBufferPool::allocate(uint64_t size, bool cpu_visible)
  {
    /* Allocate buffer with default HW-compatible alignment of 256 bytes.
     * See https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf for more. */
    return this->allocate_aligned(size, 256, cpu_visible);
  }

  gpu::MTLBuffer *MTLBufferPool::allocate_with_data(uint64_t size,
                                                    bool cpu_visible,
                                                    const void *data)
  {
    /* Allocate buffer with default HW-compatible alignment of 256 bytes.
     * See https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf for more. */
    return this->allocate_aligned_with_data(size, 256, cpu_visible, data);
  }

  gpu::MTLBuffer *MTLBufferPool::allocate_aligned(uint64_t size,
                                                  uint32_t alignment,
                                                  bool cpu_visible)
  {
    /* Check not required. Main GPU module usage considered thread-safe. */
    // KLI_assert(KLI_thread_is_main());

    /* Calculate aligned size */
    KLI_assert(alignment > 0);
    uint64_t aligned_alloc_size = ceil_to_multiple_ul(size, alignment);

    /* Allocate new MTL Buffer */
    MTL::ResourceOptions options;
    if (cpu_visible) {
      options = (m_device->hasUnifiedMemory()) ? MTL::ResourceStorageModeShared :
                                                 MTL::ResourceStorageModeManaged;
    } else {
      options = MTL::ResourceStorageModePrivate;
    }

    /* Check if we have a suitable buffer */
    gpu::MTLBuffer *new_buffer = nullptr;
    std::multiset<MTLBufferHandle, CompareMTLBuffer> **pool_search = m_buffer_pools.lookup_ptr(
      (uint64_t)options);

    if (pool_search != nullptr) {
      std::multiset<MTLBufferHandle, CompareMTLBuffer> *pool = *pool_search;
      MTLBufferHandle size_compare(aligned_alloc_size);
      auto result = pool->lower_bound(size_compare);
      if (result != pool->end()) {
        /* Potential buffer found, check if within size threshold requirements. */
        gpu::MTLBuffer *found_buffer = result->buffer;
        KLI_assert(found_buffer);
        KLI_assert(found_buffer->get_metal_buffer());

        uint64_t found_size = found_buffer->get_size();

        if (found_size >= aligned_alloc_size &&
            found_size <= (aligned_alloc_size * m_mtl_buffer_size_threshold_factor)) {
          MTL_LOG_INFO(
            "[MemoryAllocator] Suitable Buffer of size %lld found, for requested size: %lld\n",
            found_size,
            aligned_alloc_size);

          new_buffer = found_buffer;
          KLI_assert(!new_buffer->get_in_use());

          /* Remove buffer from free set. */
          pool->erase(result);
        } else {
          MTL_LOG_INFO(
            "[MemoryAllocator] Buffer of size %lld found, but was incompatible with requested "
            "size: "
            "%lld\n",
            found_size,
            aligned_alloc_size);
          new_buffer = nullptr;
        }
      }
    }

    /* Allocate new buffer. */
    if (new_buffer == nullptr) {
      new_buffer = new gpu::MTLBuffer(m_device, size, options, alignment);

      /* Track allocation in context. */
      m_allocations.append(new_buffer);
      m_total_allocation_bytes += aligned_alloc_size;
    } else {
      /* Re-use suitable buffer. */
      new_buffer->set_usage_size(aligned_alloc_size);

#if MTL_DEBUG_MEMORY_STATISTICS == 1
      /* Debug. */
      m_allocations_in_pool -= new_buffer->get_size();
      m_buffers_in_pool--;
      KLI_assert(m_allocations_in_pool >= 0);
#endif

      /* Ensure buffer memory is correctly backed. */
      KLI_assert(new_buffer->get_metal_buffer());
    }
    /* Flag buffer as actively in-use. */
    new_buffer->flag_in_use(true);

#if MTL_DEBUG_MEMORY_STATISTICS == 1
    this->per_frame_allocation_count++;
#endif

    return new_buffer;
  }

  gpu::MTLBuffer *MTLBufferPool::allocate_aligned_with_data(uint64_t size,
                                                            uint32_t alignment,
                                                            bool cpu_visible,
                                                            const void *data)
  {
    gpu::MTLBuffer *buf = this->allocate_aligned(size, 256, cpu_visible);

    /* Upload initial data. */
    KLI_assert(data != nullptr);
    KLI_assert(!(buf->get_resource_options() & MTL::ResourceStorageModePrivate));
    KLI_assert(size <= buf->get_size());
    KLI_assert(size <= buf->get_metal_buffer()->length());
    memcpy(buf->get_host_ptr(), data, size);
    buf->flush_range(0, size);
    return buf;
  }

  bool MTLBufferPool::free_buffer(gpu::MTLBuffer *buffer)
  {
    /* Ensure buffer is flagged as in-use. I.e. has not already been returned to memory pools. */
    bool buffer_in_use = buffer->get_in_use();
    KLI_assert(buffer_in_use);
    if (buffer_in_use) {

      /* Fetch active safe pool from atomic ptr. */
      MTLSafeFreeList *current_pool = this->get_current_safe_list();

      /* Place buffer in safe_free_pool before returning to MemoryManager buffer pools. */
      KLI_assert(current_pool);
      current_pool->insert_buffer(buffer);
      buffer->flag_in_use(false);

      return true;
    }
    return false;
  }

  void MTLBufferPool::update_memory_pools()
  {
    /* Ensure thread-safe access to `completed_safelist_queue_`, which contains
     * the list of MTLSafeFreeList's whose buffers are ready to be
     * re-inserted into the Memory Manager pools. */
    m_safelist_lock.lock();

#if MTL_DEBUG_MEMORY_STATISTICS == 1
    int num_buffers_added = 0;
#endif

    /* Always free oldest MTLSafeFreeList first. */
    for (int safe_pool_free_index = 0; safe_pool_free_index < m_completed_safelist_queue.size();
         safe_pool_free_index++) {
      MTLSafeFreeList *current_pool = m_completed_safelist_queue[safe_pool_free_index];

      /* Iterate through all MTLSafeFreeList linked-chunks. */
      while (current_pool != nullptr) {
        current_pool->m_lock.lock();
        KLI_assert(current_pool);
        KLI_assert(current_pool->m_in_free_queue);
        int counter = 0;
        int size = min_ii(current_pool->m_current_list_index, MTLSafeFreeList::m_MAX_NUM_BUFFERS);

        /* Re-add all buffers within frame index to MemoryManager pools. */
        while (counter < size) {

          gpu::MTLBuffer *buf = current_pool->m_safe_free_pool[counter];

          /* Insert buffer back into open pools. */
          KLI_assert(buf->get_in_use() == false);
          this->insert_buffer_into_pool(buf->get_resource_options(), buf);
          counter++;

#if MTL_DEBUG_MEMORY_STATISTICS == 1
          num_buffers_added++;
#endif
        }

        /* Fetch next MTLSafeFreeList chunk, if any. */
        MTLSafeFreeList *next_list = nullptr;
        if (current_pool->m_has_next_pool > 0) {
          next_list = current_pool->m_next.load();
        }

        /* Delete current MTLSafeFreeList */
        current_pool->m_lock.unlock();
        delete current_pool;
        current_pool = nullptr;

        /* Move onto next chunk. */
        if (next_list != nullptr) {
          current_pool = next_list;
        }
      }
    }

#if MTL_DEBUG_MEMORY_STATISTICS == 1
    printf("--- Allocation Stats ---\n");
    printf("  Num buffers processed in pool (this frame): %u\n", num_buffers_added);

    uint framealloc = (uint)this->per_frame_allocation_count;
    printf("  Allocations in frame: %u\n", framealloc);
    printf("  Total Buffers allocated: %u\n", (uint)m_allocations.size());
    printf("  Total Memory allocated: %u MB\n", (uint)m_total_allocation_bytes / (1024 * 1024));

    uint allocs = (uint)(m_allocations_in_pool) / 1024 / 2024;
    printf("  Free memory in pools: %u MB\n", allocs);

    uint buffs = (uint)m_buffers_in_pool;
    printf("  Buffers in pools: %u\n", buffs);

    printf("  Pools %u:\n", (uint)m_buffer_pools.size());
    auto key_iterator = m_buffer_pools.keys().begin();
    auto value_iterator = m_buffer_pools.values().begin();
    while (key_iterator != m_buffer_pools.keys().end()) {
      uint64_t mem_in_pool = 0;
      uint64_t iters = 0;
      for (auto it = (*value_iterator)->begin(); it != (*value_iterator)->end(); it++) {
        mem_in_pool += it->buffer_size;
        iters++;
      }

      printf("    Buffers in pool (%u)(%llu): %u (%u MB)\n",
             (uint)*key_iterator,
             iters,
             (uint)((*value_iterator)->size()),
             (uint)mem_in_pool / 1024 / 1024);
      ++key_iterator;
      ++value_iterator;
    }

    this->per_frame_allocation_count = 0;
#endif

    /* Clear safe pools list */
    m_completed_safelist_queue.clear();
    m_safelist_lock.unlock();
  }

  void MTLBufferPool::push_completed_safe_list(MTLSafeFreeList *safe_list)
  {
    /* When an MTLSafeFreeList has been released by the GPU, and buffers are ready to
     * be re-inserted into the MemoryManager pools for future use, add the MTLSafeFreeList
     * to the `completed_safelist_queue_` for flushing at a controlled point in time. */
    safe_list->m_lock.lock();
    KLI_assert(safe_list);
    KLI_assert(safe_list->m_reference_count == 0 &&
               "Pool must be fully dereferenced by all in-use cmd buffers before returning.\n");
    KLI_assert(safe_list->m_in_free_queue == false && "Pool must not already be in queue");

    /* Flag MTLSafeFreeList as having been added, and insert into SafeFreePool queue. */
    safe_list->flag_in_queue();
    m_safelist_lock.lock();
    m_completed_safelist_queue.append(safe_list);
    m_safelist_lock.unlock();
    safe_list->m_lock.unlock();
  }

  MTLSafeFreeList *MTLBufferPool::get_current_safe_list()
  {
    /* Thread-safe access via atomic ptr. */
    return m_current_free_list;
  }

  void MTLBufferPool::begin_new_safe_list()
  {
    m_safelist_lock.lock();
    m_current_free_list = new MTLSafeFreeList();
    m_safelist_lock.unlock();
  }

  void MTLBufferPool::ensure_buffer_pool(MTL::ResourceOptions options)
  {
    std::multiset<MTLBufferHandle, CompareMTLBuffer> **pool_search = m_buffer_pools.lookup_ptr(
      (uint64_t)options);
    if (pool_search == nullptr) {
      std::multiset<MTLBufferHandle, CompareMTLBuffer> *pool =
        new std::multiset<MTLBufferHandle, CompareMTLBuffer>();
      m_buffer_pools.add_new((uint64_t)options, pool);
    }
  }

  void MTLBufferPool::insert_buffer_into_pool(MTL::ResourceOptions options, gpu::MTLBuffer *buffer)
  {
    /* Ensure `safelist_lock_` is locked in calling code before modifying. */
    KLI_assert(buffer);

    /* Reset usage size to actual size of allocation. */
    buffer->set_usage_size(buffer->get_size());

    /* Ensure pool exists. */
    this->ensure_buffer_pool(options);

    /* TODO(Metal): Support purgeability - Allow buffer in pool to have its memory taken back by
     * the OS if needed. As we keep allocations around, they may not actually be in use, but we can
     * ensure they do not block other apps from using memory. Upon a buffer being needed again, we
     * can reset this state.
     *  TODO(Metal): Purgeability state does not update instantly, so this requires a deferral. */
    KLI_assert(buffer->get_metal_buffer());
    /* buffer->metal_buffer); [buffer->metal_buffer setPurgeableState:MTLPurgeableStateVolatile];
     */

    std::multiset<MTLBufferHandle, CompareMTLBuffer> *pool = m_buffer_pools.lookup(options);
    pool->insert(MTLBufferHandle(buffer));

#if MTL_DEBUG_MEMORY_STATISTICS == 1
    /* Debug statistics. */
    m_allocations_in_pool += buffer->get_size();
    m_buffers_in_pool++;
#endif
  }

  MTLSafeFreeList::MTLSafeFreeList()
  {
    m_reference_count = 1;
    m_in_free_queue = false;
    m_current_list_index = 0;
    m_next = nullptr;
    m_has_next_pool = 0;
  }

  void MTLSafeFreeList::insert_buffer(gpu::MTLBuffer *buffer)
  {
    KLI_assert(m_in_free_queue == false);

    /* Lockless list insert. */
    uint insert_index = m_current_list_index++;

    /* If the current MTLSafeFreeList size is exceeded, we ripple down the linked-list chain and
     * insert the buffer into the next available chunk. */
    if (insert_index >= MTLSafeFreeList::m_MAX_NUM_BUFFERS) {

      /* Check if first caller to generate next pool. */
      int has_next = m_has_next_pool++;
      if (has_next == 0) {
        m_next = new MTLSafeFreeList();
      }
      MTLSafeFreeList *next_list = m_next.load();
      KLI_assert(next_list);
      next_list->insert_buffer(buffer);

      /* Clamp index to chunk limit if overflowing. */
      m_current_list_index = MTLSafeFreeList::m_MAX_NUM_BUFFERS;
      return;
    }

    m_safe_free_pool[insert_index] = buffer;
  }

  /* Increments from active GPUContext thread. */
  void MTLSafeFreeList::increment_reference()
  {
    m_lock.lock();
    KLI_assert(m_in_free_queue == false);
    m_reference_count++;
    m_lock.unlock();
  }

  /* Reference decrements and addition to completed list queue can occur from MTLCommandBuffer
   * completion callback thread. */
  void MTLSafeFreeList::decrement_reference()
  {
    m_lock.lock();
    KLI_assert(m_in_free_queue == false);
    int ref_count = --m_reference_count;

    if (ref_count == 0) {
      MTLContext::get_global_memory_manager().push_completed_safe_list(this);
    }
    m_lock.unlock();
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name MTLBuffer wrapper class implementation.
   * \{ */

  /* Construct a gpu::MTLBuffer wrapper around a newly created metal::MTLBuffer. */
  MTLBuffer::MTLBuffer(MTL::Device *mtl_device,
                       uint64_t size,
                       MTL::ResourceOptions options,
                       uint alignment)
  {
    /* Calculate aligned allocation size. */
    KLI_assert(alignment > 0);
    uint64_t aligned_alloc_size = ceil_to_multiple_ul(size, alignment);

    m_alignment = alignment;
    m_device = mtl_device;
    m_is_external = false;

    m_options = options;
    this->flag_in_use(false);

    m_metal_buffer = m_device->newBuffer(aligned_alloc_size, options);
    KLI_assert(m_metal_buffer);
    m_metal_buffer->retain();

    m_size = aligned_alloc_size;
    this->set_usage_size(m_size);
    if (!(m_options & MTL::ResourceStorageModePrivate)) {
      m_data = m_metal_buffer->contents();
    } else {
      m_data = nullptr;
    }
  }

  MTLBuffer::MTLBuffer(MTL::Buffer *external_buffer)
  {
    KLI_assert(external_buffer != nil);

    /* Ensure external_buffer remains referenced while in-use. */
    m_metal_buffer = external_buffer;
    m_metal_buffer->retain();

    /* Extract properties. */
    m_is_external = true;
    m_device = nil;
    m_alignment = 1;
    m_options = m_metal_buffer->resourceOptions();
    m_size = m_metal_buffer->allocatedSize();
    this->set_usage_size(m_size);
    m_data = m_metal_buffer->contents();
    m_in_use = true;
  }

  gpu::MTLBuffer::~MTLBuffer()
  {
    if (m_metal_buffer != nil) {
      m_metal_buffer->release();
      m_metal_buffer = nil;
    }
  }

  void gpu::MTLBuffer::free()
  {
    if (!m_is_external) {
      MTLContext::get_global_memory_manager().free_buffer(this);
    } else {
      if (m_metal_buffer != nil) {
        m_metal_buffer->release();
        m_metal_buffer = nil;
      }
    }
  }

  MTL::Buffer *gpu::MTLBuffer::get_metal_buffer() const
  {
    return m_metal_buffer;
  }

  void *gpu::MTLBuffer::get_host_ptr() const
  {
    KLI_assert(!(m_options & MTL::ResourceStorageModePrivate));
    KLI_assert(m_data);
    return m_data;
  }

  uint64_t gpu::MTLBuffer::get_size() const
  {
    return m_size;
  }

  uint64_t gpu::MTLBuffer::get_size_used() const
  {
    return m_usage_size;
  }

  bool gpu::MTLBuffer::requires_flush()
  {
    /* We do not need to flush shared memory, as addressable buffer is shared. */
    return m_options & MTL::ResourceStorageModeManaged;
  }

  void gpu::MTLBuffer::set_label(NS::String *str)
  {
    m_metal_buffer->setLabel(str);
  }

  void gpu::MTLBuffer::debug_ensure_used()
  {
    /* Debug: If buffer is not flagged as in-use, this is a problem. */
    KLI_assert_msg(
      m_in_use,
      "Buffer should be marked as 'in-use' if being actively used by an instance. Buffer "
      "has likely already been freed.");
  }

  void gpu::MTLBuffer::flush()
  {
    this->debug_ensure_used();
    if (this->requires_flush()) {
      m_metal_buffer->didModifyRange(NS::Range::Make(0, m_size));
    }
  }

  void gpu::MTLBuffer::flush_range(uint64_t offset, uint64_t length)
  {
    this->debug_ensure_used();
    if (this->requires_flush()) {
      KLI_assert((offset + length) <= m_size);
      m_metal_buffer->didModifyRange(NS::Range::Make(offset, length));
    }
  }

  void gpu::MTLBuffer::flag_in_use(bool used)
  {
    m_in_use = used;
  }

  bool gpu::MTLBuffer::get_in_use()
  {
    return m_in_use;
  }

  void gpu::MTLBuffer::set_usage_size(uint64_t size_used)
  {
    KLI_assert(size_used > 0 && size_used <= m_size);
    m_usage_size = size_used;
  }

  MTL::ResourceOptions gpu::MTLBuffer::get_resource_options()
  {
    return m_options;
  }

  uint64_t gpu::MTLBuffer::get_alignment()
  {
    return m_alignment;
  }

  bool MTLBufferRange::requires_flush()
  {
    /* We do not need to flush shared memory. */
    return this->options & MTL::ResourceStorageModeManaged;
  }

  void MTLBufferRange::flush()
  {
    if (this->requires_flush()) {
      KLI_assert(this->metal_buffer);
      KLI_assert((this->buffer_offset + this->size) <= this->metal_buffer->length());
      KLI_assert(this->buffer_offset >= 0);
      this->metal_buffer->didModifyRange(
        NS::Range::Make(this->buffer_offset, this->size - this->buffer_offset));
    }
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name MTLScratchBufferManager and MTLCircularBuffer implementation.
   * \{ */

  MTLScratchBufferManager::~MTLScratchBufferManager()
  {
    this->free();
  }

  void MTLScratchBufferManager::init()
  {

    if (!this->m_initialised) {
      KLI_assert(m_context.device);

      /* Initialize Scratch buffers. */
      for (int sb = 0; sb < m_mtl_max_scratch_buffers; sb++) {
        m_scratch_buffers[sb] = new MTLCircularBuffer(m_context,
                                                      m_mtl_scratch_buffer_initial_size,
                                                      true);
        KLI_assert(m_scratch_buffers[sb]);
        KLI_assert(&(m_scratch_buffers[sb]->m_own_context) == &m_context);
      }
      m_current_scratch_buffer = 0;
      m_initialised = true;
    }
  }

  void MTLScratchBufferManager::free()
  {
    m_initialised = false;

    /* Release Scratch buffers */
    for (int sb = 0; sb < m_mtl_max_scratch_buffers; sb++) {
      delete m_scratch_buffers[sb];
      m_scratch_buffers[sb] = nullptr;
    }
    m_current_scratch_buffer = 0;
  }

  MTLTemporaryBuffer MTLScratchBufferManager::scratch_buffer_allocate_range(uint64_t alloc_size)
  {
    return this->scratch_buffer_allocate_range_aligned(alloc_size, 1);
  }

  MTLTemporaryBuffer MTLScratchBufferManager::scratch_buffer_allocate_range_aligned(
    uint64_t alloc_size,
    uint alignment)
  {
    /* Ensure scratch buffer allocation alignment adheres to offset alignment requirements. */
    alignment = max_uu(alignment, 256);

    KLI_assert_msg(m_current_scratch_buffer >= 0, "Scratch Buffer index not set");
    MTLCircularBuffer *current_scratch_buff = this->m_scratch_buffers[m_current_scratch_buffer];
    KLI_assert_msg(current_scratch_buff != nullptr, "Scratch Buffer does not exist");
    MTLTemporaryBuffer allocated_range = current_scratch_buff->allocate_range_aligned(alloc_size,
                                                                                      alignment);
    KLI_assert(allocated_range.size >= alloc_size &&
               allocated_range.size <= alloc_size + alignment);
    KLI_assert(allocated_range.metal_buffer != nil);
    return allocated_range;
  }

  void MTLScratchBufferManager::ensure_increment_scratch_buffer()
  {
    /* Fetch active scratch buffer. */
    MTLCircularBuffer *active_scratch_buf = m_scratch_buffers[m_current_scratch_buffer];
    KLI_assert(&active_scratch_buf->m_own_context == &m_context);

    /* Ensure existing scratch buffer is no longer in use. MTL_MAX_SCRATCH_BUFFERS specifies
     * the number of allocated scratch buffers. This value should be equal to the number of
     * simultaneous frames in-flight. I.e. the maximal number of scratch buffers which are
     * simultaneously in-use. */
    if (active_scratch_buf->m_used_frame_index < m_context.get_current_frame_index()) {
      m_current_scratch_buffer = (m_current_scratch_buffer + 1) % m_mtl_max_scratch_buffers;
      active_scratch_buf = m_scratch_buffers[m_current_scratch_buffer];
      active_scratch_buf->reset();
      KLI_assert(&active_scratch_buf->m_own_context == &m_context);
      MTL_LOG_INFO("Scratch buffer %d reset - (ctx %p)(Frame index: %d)\n",
                   m_current_scratch_buffer,
                   &m_context,
                   m_context.get_current_frame_index());
    }
  }

  void MTLScratchBufferManager::flush_active_scratch_buffer()
  {
    /* Fetch active scratch buffer and verify context. */
    MTLCircularBuffer *active_scratch_buf = m_scratch_buffers[m_current_scratch_buffer];
    KLI_assert(&active_scratch_buf->m_own_context == &m_context);
    active_scratch_buf->flush();
  }

  /* MTLCircularBuffer implementation. */
  MTLCircularBuffer::MTLCircularBuffer(MTLContext &ctx, uint64_t initial_size, bool allow_grow)
    : m_own_context(ctx)
  {
    KLI_assert(this);
    MTL::ResourceOptions options = (m_own_context.device->hasUnifiedMemory()) ?
                                     MTL::ResourceStorageModeShared :
                                     MTL::ResourceStorageModeManaged;
    m_cbuffer = new gpu::MTLBuffer(m_own_context.device, initial_size, options, 256);
    m_current_offset = 0;
    m_can_resize = allow_grow;
    m_cbuffer->flag_in_use(true);

    m_used_frame_index = ctx.get_current_frame_index();
    m_last_flush_base_offset = 0;

    /* Debug label. */
    if (G.debug & G_DEBUG_GPU) {
      m_cbuffer->set_label(NS_STRING_("Circular Scratch Buffer"));
    }
  }

  MTLCircularBuffer::~MTLCircularBuffer()
  {
    delete m_cbuffer;
  }

  MTLTemporaryBuffer MTLCircularBuffer::allocate_range(uint64_t alloc_size)
  {
    return this->allocate_range_aligned(alloc_size, 1);
  }

  MTLTemporaryBuffer MTLCircularBuffer::allocate_range_aligned(uint64_t alloc_size, uint alignment)
  {
    KLI_assert(this);

    /* Ensure alignment of an allocation is aligned to compatible offset boundaries. */
    KLI_assert(alignment > 0);
    alignment = max_ulul(alignment, 256);

    /* Align current offset and allocation size to desired alignment */
    uint64_t aligned_current_offset = ceil_to_multiple_ul(m_current_offset, alignment);
    uint64_t aligned_alloc_size = ceil_to_multiple_ul(alloc_size, alignment);
    bool can_allocate = (aligned_current_offset + aligned_alloc_size) < m_cbuffer->get_size();

    KLI_assert(aligned_current_offset >= m_current_offset);
    KLI_assert(aligned_alloc_size >= alloc_size);

    KLI_assert(aligned_current_offset % alignment == 0);
    KLI_assert(aligned_alloc_size % alignment == 0);

    /* Recreate Buffer */
    if (!can_allocate) {
      uint64_t new_size = m_cbuffer->get_size();
      if (m_can_resize) {
        /* Resize to the maximum of basic resize heuristic OR the size of the current offset +
         * requested allocation -- we want the buffer to grow to a large enough size such that it
         * does not need to resize mid-frame. */
        new_size = max_ulul(
          min_ulul(MTLScratchBufferManager::m_mtl_scratch_buffer_max_size, new_size * 1.2),
          aligned_current_offset + aligned_alloc_size);

#if MTL_SCRATCH_BUFFER_ALLOW_TEMPORARY_EXPANSION == 1
        /* IF a requested allocation EXCEEDS the maximum supported size, temporarily allocate up to
         * this, but shrink down ASAP. */
        if (new_size > MTLScratchBufferManager::m_mtl_scratch_buffer_max_size) {

          /* If new requested allocation is bigger than maximum allowed size, temporarily resize to
           * maximum allocation size -- Otherwise, clamp the buffer size back down to the defined
           * maximum */
          if (aligned_alloc_size > MTLScratchBufferManager::m_mtl_scratch_buffer_max_size) {
            new_size = aligned_alloc_size;
            MTL_LOG_INFO("Temporarily growing Scratch buffer to %d MB\n",
                         (int)new_size / 1024 / 1024);
          } else {
            new_size = MTLScratchBufferManager::m_mtl_scratch_buffer_max_size;
            MTL_LOG_INFO("Shrinking Scratch buffer back to %d MB\n", (int)new_size / 1024 / 1024);
          }
        }
        KLI_assert(aligned_alloc_size <= new_size);
#else
        new_size = min_ulul(MTLScratchBufferManager::m_mtl_scratch_buffer_max_size, new_size);

        if (aligned_alloc_size > new_size) {
          KLI_assert(false);

          /* Cannot allocate */
          MTLTemporaryBuffer alloc_range;
          alloc_range.metal_buffer = nil;
          alloc_range.data = nullptr;
          alloc_range.buffer_offset = 0;
          alloc_range.size = 0;
          alloc_range.options = m_cbuffer->options;
        }
#endif
      } else {
        MTL_LOG_WARNING(
          "Performance Warning: Reached the end of circular buffer of size: %llu, but cannot "
          "resize. Starting new buffer\n",
          m_cbuffer->get_size());
        KLI_assert(aligned_alloc_size <= new_size);

        /* Cannot allocate. */
        MTLTemporaryBuffer alloc_range;
        alloc_range.metal_buffer = nil;
        alloc_range.data = nullptr;
        alloc_range.buffer_offset = 0;
        alloc_range.size = 0;
        alloc_range.options = m_cbuffer->get_resource_options();
      }

      /* Flush current buffer to ensure changes are visible on the GPU. */
      this->flush();

      /* Discard old buffer and create a new one - Relying on Metal reference counting to track
       * in-use buffers */
      MTL::ResourceOptions prev_options = m_cbuffer->get_resource_options();
      uint prev_alignment = m_cbuffer->get_alignment();
      delete m_cbuffer;
      m_cbuffer = new gpu::MTLBuffer(m_own_context.device, new_size, prev_options, prev_alignment);
      m_cbuffer->flag_in_use(true);
      m_current_offset = 0;
      m_last_flush_base_offset = 0;

      /* Debug label. */
      if (G.debug & G_DEBUG_GPU) {
        m_cbuffer->set_label(NS_STRING_("Circular Scratch Buffer"));
      }
      MTL_LOG_INFO("Resized Metal circular buffer to %llu bytes\n", new_size);

      /* Reset allocation Status. */
      aligned_current_offset = 0;
      KLI_assert((aligned_current_offset + aligned_alloc_size) <= m_cbuffer->get_size());
    }

    /* Allocate chunk. */
    MTLTemporaryBuffer alloc_range;
    alloc_range.metal_buffer = m_cbuffer->get_metal_buffer();
    alloc_range.data = (void *)((uint8_t *)(alloc_range.metal_buffer->contents()) +
                                aligned_current_offset);
    alloc_range.buffer_offset = aligned_current_offset;
    alloc_range.size = aligned_alloc_size;
    alloc_range.options = m_cbuffer->get_resource_options();
    KLI_assert(alloc_range.data);

    /* Shift offset to match alignment. */
    m_current_offset = aligned_current_offset + aligned_alloc_size;
    KLI_assert(m_current_offset <= m_cbuffer->get_size());
    return alloc_range;
  }

  void MTLCircularBuffer::flush()
  {
    KLI_assert(this);

    uint64_t len = m_current_offset - m_last_flush_base_offset;
    if (len > 0) {
      m_cbuffer->flush_range(m_last_flush_base_offset, len);
      m_last_flush_base_offset = m_current_offset;
    }
  }

  void MTLCircularBuffer::reset()
  {
    KLI_assert(this);

    /* If circular buffer has data written to it, offset will be greater than zero. */
    if (m_current_offset > 0) {

      /* Ensure the circular buffer is no longer being used by an in-flight frame. */
      KLI_assert((m_own_context.get_current_frame_index() >=
                  (m_used_frame_index + MTL_NUM_SAFE_FRAMES - 1)) &&
                 "Trying to reset Circular scratch buffer's while its data is still being used by "
                 "an in-flight frame");

      m_current_offset = 0;
      m_last_flush_base_offset = 0;
    }

    /* Update used frame index to current. */
    m_used_frame_index = m_own_context.get_current_frame_index();
  }

  /** @} */

}  // namespace kraken::gpu
