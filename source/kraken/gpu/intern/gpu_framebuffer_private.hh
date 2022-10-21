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

#include "KLI_math_vector.h"
#include "KLI_span.hh"

#include "MEM_guardedalloc.h"

#include "GPU_framebuffer.h"

struct GPUTexture;

typedef enum GPUAttachmentType : int
{
  GPU_FB_DEPTH_ATTACHMENT = 0,
  GPU_FB_DEPTH_STENCIL_ATTACHMENT,
  GPU_FB_COLOR_ATTACHMENT0,
  GPU_FB_COLOR_ATTACHMENT1,
  GPU_FB_COLOR_ATTACHMENT2,
  GPU_FB_COLOR_ATTACHMENT3,
  GPU_FB_COLOR_ATTACHMENT4,
  GPU_FB_COLOR_ATTACHMENT5,
  GPU_FB_COLOR_ATTACHMENT6,
  GPU_FB_COLOR_ATTACHMENT7,
  /* Number of maximum output slots. */
  /* Keep in mind that GL max is GL_MAX_DRAW_BUFFERS and is at least 8, corresponding to
   * the maximum number of COLOR attachments specified by glDrawBuffers. */
  GPU_FB_MAX_ATTACHMENT,

} GPUAttachmentType;

#define GPU_FB_MAX_COLOR_ATTACHMENT (GPU_FB_MAX_ATTACHMENT - GPU_FB_COLOR_ATTACHMENT0)

inline constexpr GPUAttachmentType operator-(GPUAttachmentType a, int b)
{
  return static_cast<GPUAttachmentType>(int(a) - b);
}

inline constexpr GPUAttachmentType operator+(GPUAttachmentType a, int b)
{
  return static_cast<GPUAttachmentType>(int(a) + b);
}

inline GPUAttachmentType &operator++(GPUAttachmentType &a)
{
  a = a + 1;
  return a;
}

inline GPUAttachmentType &operator--(GPUAttachmentType &a)
{
  a = a - 1;
  return a;
}

namespace kraken
{
  namespace gpu
  {

#ifdef DEBUG
#  define DEBUG_NAME_LEN 64
#else
#  define DEBUG_NAME_LEN 16
#endif

    class FrameBuffer
    {
     protected:

      /** Set of texture attachments to render to. DEPTH and DEPTH_STENCIL are mutually exclusive.
       */
      GPUAttachment m_attachments[GPU_FB_MAX_ATTACHMENT];
      /** Is true if internal representation need to be updated. */
      bool m_dirty_attachments = true;
      /** Size of attachment textures. */
      int m_width = 0, m_height = 0;
      /** Debug name. */
      char m_name[DEBUG_NAME_LEN];
      /** Frame-buffer state. */
      int m_viewport[4] = {0};
      int m_scissor[4] = {0};
      bool m_scissor_test = false;
      bool m_dirty_state = true;

#ifndef GPU_NO_USE_PY_REFERENCES

     public:

      /**
       * Reference of a pointer that needs to be cleaned when deallocating the frame-buffer.
       * Points to #BPyGPUFrameBuffer.fb
       */
      void **py_ref = nullptr;
#endif

     public:

      /* Reference of a pointer that needs to be cleaned when deallocating the frame-buffer.
       * Points to #BPyGPUFrameBuffer::fb */
      void **ref = nullptr;

     public:

      FrameBuffer(const char *name);
      virtual ~FrameBuffer();

      virtual void bind(bool enabled_srgb) = 0;
      virtual bool check(char err_out[256]) = 0;
      virtual void clear(eGPUFrameBufferBits buffers,
                         const float clear_col[4],
                         float clear_depth,
                         uint clear_stencil) = 0;
      virtual void clear_multi(const float (*clear_col)[4]) = 0;
      virtual void clear_attachment(GPUAttachmentType type,
                                    eGPUDataFormat data_format,
                                    const void *clear_value) = 0;

      virtual void attachment_set_loadstore_op(GPUAttachmentType type,
                                               eGPULoadOp load_action,
                                               eGPUStoreOp store_action) = 0;

      virtual void read(eGPUFrameBufferBits planes,
                        eGPUDataFormat format,
                        const int area[4],
                        int channel_len,
                        int slot,
                        void *r_data) = 0;

      virtual void blit_to(eGPUFrameBufferBits planes,
                           int src_slot,
                           FrameBuffer *dst,
                           int dst_slot,
                           int dst_offset_x,
                           int dst_offset_y) = 0;

      void load_store_config_array(const GPULoadStore *load_store_actions, uint actions_len);

      void attachment_set(GPUAttachmentType type, const GPUAttachment &new_attachment);
      void attachment_remove(GPUAttachmentType type);

      void recursive_downsample(int max_lvl,
                                void (*callback)(void *userData, int level),
                                void *userData);
      uint get_bits_per_pixel();

      inline void size_set(int width, int height)
      {
        m_width = width;
        m_height = height;
        m_dirty_state = true;
      }

      inline void viewport_set(const int viewport[4])
      {
        if (!equals_v4v4_int(m_viewport, viewport)) {
          copy_v4_v4_int(m_viewport, viewport);
          m_dirty_state = true;
        }
      }

      inline void scissor_set(const int scissor[4])
      {
        if (!equals_v4v4_int(m_scissor, scissor)) {
          copy_v4_v4_int(m_scissor, scissor);
          m_dirty_state = true;
        }
      }

      inline void scissor_test_set(bool test)
      {
        m_scissor_test = test;
      }

      inline void viewport_get(int r_viewport[4]) const
      {
        copy_v4_v4_int(r_viewport, m_viewport);
      }

      inline void scissor_get(int r_scissor[4]) const
      {
        copy_v4_v4_int(r_scissor, m_scissor);
      }

      inline bool scissor_test_get() const
      {
        return m_scissor_test;
      }

      inline void viewport_reset()
      {
        int viewport_rect[4] = {0, 0, m_width, m_height};
        viewport_set(viewport_rect);
      }

      inline void scissor_reset()
      {
        int scissor_rect[4] = {0, 0, m_width, m_height};
        scissor_set(scissor_rect);
      }

      inline GPUTexture *depth_tex() const
      {
        if (m_attachments[GPU_FB_DEPTH_ATTACHMENT].tex) {
          return m_attachments[GPU_FB_DEPTH_ATTACHMENT].tex;
        }
        return m_attachments[GPU_FB_DEPTH_STENCIL_ATTACHMENT].tex;
      };

      inline GPUTexture *color_tex(int slot) const
      {
        return m_attachments[GPU_FB_COLOR_ATTACHMENT0 + slot].tex;
      };
    };

    /* Syntactic sugar. */
    static inline GPUFrameBuffer *wrap(FrameBuffer *vert)
    {
      return reinterpret_cast<GPUFrameBuffer *>(vert);
    }
    static inline FrameBuffer *unwrap(GPUFrameBuffer *vert)
    {
      return reinterpret_cast<FrameBuffer *>(vert);
    }
    static inline const FrameBuffer *unwrap(const GPUFrameBuffer *vert)
    {
      return reinterpret_cast<const FrameBuffer *>(vert);
    }

#undef DEBUG_NAME_LEN

  }  // namespace gpu
}  // namespace kraken
