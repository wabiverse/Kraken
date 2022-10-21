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

#include "mtl_context.hh"
#include "mtl_debug.hh"
#include "mtl_framebuffer.hh"
#include "mtl_texture.hh"

#include <wabi/base/arch/defines.h>

namespace kraken::gpu
{

  /* -------------------------------------------------------------------- */
  /** @name Creation & Deletion
   * @{ */

  MTLFrameBuffer::MTLFrameBuffer(MTLContext *ctx, const char *name) : FrameBuffer(name)
  {

    m_context = ctx;
    m_is_dirty = true;
    m_is_loadstore_dirty = true;
    m_dirty_state_ctx = nullptr;
    m_has_pending_clear = false;
    m_colour_attachment_count = 0;
    m_srgb_enabled = false;
    m_is_srgb = false;

    for (int i = 0; i < GPU_FB_MAX_COLOR_ATTACHMENT; i++) {
      m_mtl_color_attachments[i].used = false;
    }
    m_mtl_depth_attachment.used = false;
    m_mtl_stencil_attachment.used = false;

    for (int i = 0; i < MTL_FB_CONFIG_MAX; i++) {
      m_framebuffer_descriptor[i] = MTL::RenderPassDescriptor::alloc()->init();
      m_descriptor_dirty[i] = true;
    }

    for (int i = 0; i < GPU_FB_MAX_COLOR_ATTACHMENT; i++) {
      m_colour_attachment_descriptors[i] =
        MTL::RenderPassColorAttachmentDescriptor::alloc()->init();
    }

    /* Initial state. */
    this->size_set(0, 0);
    this->viewport_reset();
    this->scissor_reset();
  }

  MTLFrameBuffer::~MTLFrameBuffer()
  {
    /* If FrameBuffer is associated with a currently open RenderPass, end. */
    if (m_context->main_command_buffer.get_active_framebuffer() == this) {
      m_context->main_command_buffer.end_active_command_encoder();
    }

    /* Restore default frame-buffer if this frame-buffer was bound. */
    if (m_context->active_fb == this && m_context->back_left != this) {
      /* If this assert triggers it means the frame-buffer is being freed while in use by another
       * context which, by the way, is TOTALLY UNSAFE!!!  (Copy from GL behavior). */
      KLI_assert(m_context == static_cast<MTLContext *>(unwrap(GPU_context_active_get())));
      GPU_framebuffer_restore();
    }

    /* Free Render Pass Descriptors. */
    for (int config = 0; config < MTL_FB_CONFIG_MAX; config++) {
      if (m_framebuffer_descriptor[config] != nil) {
        m_framebuffer_descriptor[config]->release();
        m_framebuffer_descriptor[config] = nil;
      }
    }

    /* Free colour attachment descriptors. */
    for (int i = 0; i < GPU_FB_MAX_COLOR_ATTACHMENT; i++) {
      if (m_colour_attachment_descriptors[i] != nil) {
        m_colour_attachment_descriptors[i]->release();
        m_colour_attachment_descriptors[i] = nil;
      }
    }

    /* Remove attachments - release FB texture references. */
    this->remove_all_attachments();

    if (m_context == nullptr) {
      return;
    }
  }

  void MTLFrameBuffer::bind(bool enabled_srgb)
  {

    /* Verify Context is valid. */
    if (m_context != static_cast<MTLContext *>(unwrap(GPU_context_active_get()))) {
      KLI_assert(false && "Trying to use the same frame-buffer in multiple context's.");
      return;
    }

    /* Ensure SRGB state is up-to-date and valid. */
    bool srgb_state_changed = m_srgb_enabled != enabled_srgb;
    if (m_context->active_fb != this || srgb_state_changed) {
      if (srgb_state_changed) {
        this->mark_dirty();
      }
      m_srgb_enabled = enabled_srgb;
      GPU_shader_set_framebuffer_srgb_target(m_srgb_enabled && m_is_srgb);
    }

    /* Ensure local MTLAttachment data is up to date. */
    this->update_attachments(true);

    /* Reset clear state on bind -- Clears and load/store ops are set after binding. */
    this->reset_clear_state();

    /* Bind to active context. */
    MTLContext *mtl_context = reinterpret_cast<MTLContext *>(GPU_context_active_get());
    if (mtl_context) {
      mtl_context->framebuffer_bind(this);
      m_dirty_state = true;
    } else {
      MTL_LOG_WARNING("Attempting to bind FrameBuffer, but no context is active\n");
    }
  }

  bool MTLFrameBuffer::check(char err_out[256])
  {
    /* Ensure local MTLAttachment data is up to date. */
    this->update_attachments(true);

    /* Ensure there is at least one attachment. */
    bool valid = (this->get_attachment_count() > 0 ||
                  this->has_depth_attachment() | this->has_stencil_attachment());
    if (!valid) {
      const char *format = "Framebuffer %s does not have any attachments.\n";
      if (err_out) {
        KLI_snprintf(err_out, 256, format, m_name);
      } else {
        MTL_LOG_ERROR(format, m_name);
      }
      return false;
    }

    /* Ensure all attachments have identical dimensions. */
    /* Ensure all attachments are render-targets. */
    bool first = true;
    uint dim_x = 0;
    uint dim_y = 0;
    for (int col_att = 0; col_att < this->get_attachment_count(); col_att++) {
      MTLAttachment att = this->get_color_attachment(col_att);
      if (att.used) {
        if (att.texture->m_gpu_image_usage_flags & GPU_TEXTURE_USAGE_ATTACHMENT) {
          if (first) {
            dim_x = att.texture->width_get();
            dim_y = att.texture->height_get();
            first = false;
          } else {
            if (dim_x != att.texture->width_get() || dim_y != att.texture->height_get()) {
              const char *format =
                "Framebuffer %s: Color attachment dimensions do not match those of previous "
                "attachment\n";
              if (err_out) {
                KLI_snprintf(err_out, 256, format, m_name);
              } else {
                fprintf(stderr, format, m_name);
                MTL_LOG_ERROR(format, m_name);
              }
              return false;
            }
          }
        } else {
          const char *format =
            "Framebuffer %s: Color attachment texture does not have usage flag "
            "'GPU_TEXTURE_USAGE_ATTACHMENT'\n";
          if (err_out) {
            KLI_snprintf(err_out, 256, format, m_name);
          } else {
            fprintf(stderr, format, m_name);
            MTL_LOG_ERROR(format, m_name);
          }
          return false;
        }
      }
    }
    MTLAttachment depth_att = this->get_depth_attachment();
    MTLAttachment stencil_att = this->get_stencil_attachment();
    if (depth_att.used) {
      if (first) {
        dim_x = depth_att.texture->width_get();
        dim_y = depth_att.texture->height_get();
        first = false;
        valid = (depth_att.texture->m_gpu_image_usage_flags & GPU_TEXTURE_USAGE_ATTACHMENT);

        if (!valid) {
          const char *format =
            "Framebuffer %n: Depth attachment does not have usage "
            "'GPU_TEXTURE_USAGE_ATTACHMENT'\n";
          if (err_out) {
            KLI_snprintf(err_out, 256, format, m_name);
          } else {
            fprintf(stderr, format, m_name);
            MTL_LOG_ERROR(format, m_name);
          }
          return false;
        }
      } else {
        if (dim_x != depth_att.texture->width_get() || dim_y != depth_att.texture->height_get()) {
          const char *format =
            "Framebuffer %n: Depth attachment dimensions do not match that of previous "
            "attachment\n";
          if (err_out) {
            KLI_snprintf(err_out, 256, format, m_name);
          } else {
            fprintf(stderr, format, m_name);
            MTL_LOG_ERROR(format, m_name);
          }
          return false;
        }
      }
    }
    if (stencil_att.used) {
      if (first) {
        dim_x = stencil_att.texture->width_get();
        dim_y = stencil_att.texture->height_get();
        first = false;
        valid = (stencil_att.texture->m_gpu_image_usage_flags & GPU_TEXTURE_USAGE_ATTACHMENT);
        if (!valid) {
          const char *format =
            "Framebuffer %s: Stencil attachment does not have usage "
            "'GPU_TEXTURE_USAGE_ATTACHMENT'\n";
          if (err_out) {
            KLI_snprintf(err_out, 256, format, m_name);
          } else {
            fprintf(stderr, format, m_name);
            MTL_LOG_ERROR(format, m_name);
          }
          return false;
        }
      } else {
        if (dim_x != stencil_att.texture->width_get() ||
            dim_y != stencil_att.texture->height_get()) {
          const char *format =
            "Framebuffer %s: Stencil attachment dimensions do not match that of previous "
            "attachment";
          if (err_out) {
            KLI_snprintf(err_out, 256, format, m_name);
          } else {
            fprintf(stderr, format, m_name);
            MTL_LOG_ERROR(format, m_name);
          }
          return false;
        }
      }
    }

    KLI_assert(valid);
    return valid;
  }

  void MTLFrameBuffer::force_clear()
  {
    /* Perform clear by ending current and starting a new render pass. */
    MTLContext *mtl_context = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    MTLFrameBuffer *current_framebuffer = mtl_context->get_current_framebuffer();
    if (current_framebuffer) {
      KLI_assert(current_framebuffer == this);
      /* End current render-pass. */
      if (mtl_context->main_command_buffer.is_inside_render_pass()) {
        mtl_context->main_command_buffer.end_active_command_encoder();
      }
      mtl_context->ensure_begin_render_pass();
      KLI_assert(m_has_pending_clear == false);
    }
  }

  void MTLFrameBuffer::clear(eGPUFrameBufferBits buffers,
                             const float clear_col[4],
                             float clear_depth,
                             uint clear_stencil)
  {

    KLI_assert(unwrap(GPU_context_active_get()) == m_context);
    KLI_assert(m_context->active_fb == this);

    /* Ensure attachments are up to date. */
    this->update_attachments(true);

    /* If we had no previous clear pending, reset clear state. */
    if (!m_has_pending_clear) {
      this->reset_clear_state();
    }

    /* Ensure we only clear if attachments exist for given buffer bits. */
    bool do_clear = false;
    if (buffers & GPU_COLOR_BIT) {
      for (int i = 0; i < m_colour_attachment_count; i++) {
        this->set_color_attachment_clear_color(i, clear_col);
        do_clear = true;
      }
    }

    if (buffers & GPU_DEPTH_BIT) {
      this->set_depth_attachment_clear_value(clear_depth);
      do_clear = do_clear || this->has_depth_attachment();
    }
    if (buffers & GPU_STENCIL_BIT) {
      this->set_stencil_attachment_clear_value(clear_stencil);
      do_clear = do_clear || this->has_stencil_attachment();
    }

    if (do_clear) {
      m_has_pending_clear = true;

      /* Apply state before clear. */
      this->apply_state();

      /* TODO(Metal): Optimize - Currently force-clear always used. Consider moving clear state to
       * MTLTexture instead. */
      /* Force clear if RP is not yet active -- not the most efficient, but there is no distinction
       * between clears where no draws occur. Can optimize at the high-level by using explicit
       * load-store flags. */
      this->force_clear();
    }
  }

  void MTLFrameBuffer::clear_multi(const float (*clear_cols)[4])
  {
    /* If we had no previous clear pending, reset clear state. */
    if (!m_has_pending_clear) {
      this->reset_clear_state();
    }

    bool do_clear = false;
    for (int i = 0; i < this->get_attachment_limit(); i++) {
      if (this->has_attachment_at_slot(i)) {
        this->set_color_attachment_clear_color(i, clear_cols[i]);
        do_clear = true;
      }
    }

    if (do_clear) {
      m_has_pending_clear = true;

      /* Apply state before clear. */
      this->apply_state();

      /* TODO(Metal): Optimize - Currently force-clear always used. Consider moving clear state to
       * MTLTexture instead. */
      /* Force clear if RP is not yet active -- not the most efficient, but there is no distinction
       * between clears where no draws occur. Can optimize at the high-level by using explicit
       * load-store flags. */
      this->force_clear();
    }
  }

  void MTLFrameBuffer::clear_attachment(GPUAttachmentType type,
                                        eGPUDataFormat data_format,
                                        const void *clear_value)
  {
    KLI_assert(static_cast<MTLContext *>(unwrap(GPU_context_active_get())) == m_context);
    KLI_assert(m_context->active_fb == this);

    /* If we had no previous clear pending, reset clear state. */
    if (!m_has_pending_clear) {
      this->reset_clear_state();
    }

    bool do_clear = false;

    if (type == GPU_FB_DEPTH_STENCIL_ATTACHMENT) {
      if (this->has_depth_attachment() || this->has_stencil_attachment()) {
        KLI_assert(data_format == GPU_DATA_UINT_24_8);
        float depth = ((*(uint32_t *)clear_value) & 0x00FFFFFFu) / (float)0x00FFFFFFu;
        int stencil = ((*(uint32_t *)clear_value) >> 24);
        this->set_depth_attachment_clear_value(depth);
        this->set_stencil_attachment_clear_value(stencil);
        do_clear = true;
      }
    } else if (type == GPU_FB_DEPTH_ATTACHMENT) {
      if (this->has_depth_attachment()) {
        if (data_format == GPU_DATA_FLOAT) {
          this->set_depth_attachment_clear_value(*(float *)clear_value);
        } else {
          float depth = *(uint32_t *)clear_value / (float)0xFFFFFFFFu;
          this->set_depth_attachment_clear_value(depth);
        }
        do_clear = true;
      }
    } else {
      int slot = type - GPU_FB_COLOR_ATTACHMENT0;
      if (this->has_attachment_at_slot(slot)) {
        float col_clear_val[4] = {0.0};
        switch (data_format) {
          case GPU_DATA_FLOAT: {
            const float *vals = (float *)clear_value;
            col_clear_val[0] = vals[0];
            col_clear_val[1] = vals[1];
            col_clear_val[2] = vals[2];
            col_clear_val[3] = vals[3];
          } break;
          case GPU_DATA_UINT: {
            const uint *vals = (uint *)clear_value;
            col_clear_val[0] = (float)(vals[0]);
            col_clear_val[1] = (float)(vals[1]);
            col_clear_val[2] = (float)(vals[2]);
            col_clear_val[3] = (float)(vals[3]);
          } break;
          case GPU_DATA_INT: {
            const int *vals = (int *)clear_value;
            col_clear_val[0] = (float)(vals[0]);
            col_clear_val[1] = (float)(vals[1]);
            col_clear_val[2] = (float)(vals[2]);
            col_clear_val[3] = (float)(vals[3]);
          } break;
          default:
            KLI_assert_msg(0, "Unhandled data format");
            break;
        }
        this->set_color_attachment_clear_color(slot, col_clear_val);
        do_clear = true;
      }
    }

    if (do_clear) {
      m_has_pending_clear = true;

      /* Apply state before clear. */
      this->apply_state();

      /* TODO(Metal): Optimize - Currently force-clear always used. Consider moving clear state to
       * MTLTexture instead. */
      /* Force clear if RP is not yet active -- not the most efficient, but there is no distinction
       * between clears where no draws occur. Can optimize at the high-level by using explicit
       * load-store flags. */
      this->force_clear();
    }
  }

  void MTLFrameBuffer::read(eGPUFrameBufferBits planes,
                            eGPUDataFormat format,
                            const int area[4],
                            int channel_len,
                            int slot,
                            void *r_data)
  {

    KLI_assert((planes & GPU_STENCIL_BIT) == 0);
    KLI_assert(area[2] > 0);
    KLI_assert(area[3] > 0);

    switch (planes) {
      case GPU_DEPTH_BIT: {
        if (this->has_depth_attachment()) {
          MTLAttachment depth = this->get_depth_attachment();
          gpu::MTLTexture *tex = depth.texture;
          if (tex) {
            size_t sample_len = area[2] * area[3];
            size_t sample_size = to_bytesize(tex->m_format, format);
            int debug_data_size = sample_len * sample_size;
            tex->read_internal(0,
                               area[0],
                               area[1],
                               0,
                               area[2],
                               area[3],
                               1,
                               format,
                               channel_len,
                               debug_data_size,
                               r_data);
          }
        } else {
          MTL_LOG_ERROR(
            "Attempting to read depth from a framebuffer which does not have a depth "
            "attachment!\n");
        }
      }
        return;

      case GPU_COLOR_BIT: {
        if (this->has_attachment_at_slot(slot)) {
          MTLAttachment color = this->get_color_attachment(slot);
          gpu::MTLTexture *tex = color.texture;
          if (tex) {
            size_t sample_len = area[2] * area[3];
            size_t sample_size = to_bytesize(tex->m_format, format);
            int debug_data_size = sample_len * sample_size * channel_len;
            tex->read_internal(0,
                               area[0],
                               area[1],
                               0,
                               area[2],
                               area[3],
                               1,
                               format,
                               channel_len,
                               debug_data_size,
                               r_data);
          }
        }
      }
        return;

      case GPU_STENCIL_BIT:
        MTL_LOG_ERROR("GPUFramebuffer: Error: Trying to read stencil bit. Unsupported.\n");
        return;
    }
  }

  void MTLFrameBuffer::blit_to(eGPUFrameBufferBits planes,
                               int src_slot,
                               FrameBuffer *dst,
                               int dst_slot,
                               int dst_offset_x,
                               int dst_offset_y)
  {
    this->update_attachments(true);
    static_cast<MTLFrameBuffer *>(dst)->update_attachments(true);

    KLI_assert(planes != 0);

    MTLFrameBuffer *metal_fb_write = static_cast<MTLFrameBuffer *>(dst);

    KLI_assert(this);
    KLI_assert(metal_fb_write);

    /* Get width/height from attachment. */
    MTLAttachment src_attachment;
    const bool do_color = (planes & GPU_COLOR_BIT);
    const bool do_depth = (planes & GPU_DEPTH_BIT);
    const bool do_stencil = (planes & GPU_STENCIL_BIT);

    if (do_color) {
      KLI_assert(!do_depth && !do_stencil);
      src_attachment = this->get_color_attachment(src_slot);
    } else if (do_depth) {
      KLI_assert(!do_color && !do_stencil);
      src_attachment = this->get_depth_attachment();
    } else if (do_stencil) {
      KLI_assert(!do_color && !do_depth);
      src_attachment = this->get_stencil_attachment();
    }

    KLI_assert(src_attachment.used);
    this->blit(src_slot,
               0,
               0,
               metal_fb_write,
               dst_slot,
               dst_offset_x,
               dst_offset_y,
               src_attachment.texture->width_get(),
               src_attachment.texture->height_get(),
               planes);
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @ Private METAL implementation functions
   * @{ */

  void MTLFrameBuffer::mark_dirty()
  {
    m_is_dirty = true;
    m_is_loadstore_dirty = true;
  }

  void MTLFrameBuffer::mark_loadstore_dirty()
  {
    m_is_loadstore_dirty = true;
  }

  void MTLFrameBuffer::mark_cleared()
  {
    m_has_pending_clear = false;
  }

  void MTLFrameBuffer::mark_do_clear()
  {
    m_has_pending_clear = true;
  }

  void MTLFrameBuffer::update_attachments(bool update_viewport)
  {
    if (!m_dirty_attachments) {
      return;
    }

    /* Cache viewport and scissor (If we have existing attachments). */
    int t_viewport[4], t_scissor[4];
    update_viewport = update_viewport &&
                      (this->get_attachment_count() > 0 && this->has_depth_attachment() &&
                       this->has_stencil_attachment());
    if (update_viewport) {
      this->viewport_get(t_viewport);
      this->scissor_get(t_scissor);
    }

    /* Clear current attachments state. */
    this->remove_all_attachments();

    /* Reset framebuffer options. */
    m_use_multilayered_rendering = false;

    /* Track first attachment for SRGB property extraction. */
    GPUAttachmentType first_attachment = GPU_FB_MAX_ATTACHMENT;
    MTLAttachment first_attachment_mtl;

    /* Scan through changes to attachments and populate local structures. */
    bool depth_added = false;
    for (GPUAttachmentType type = GPU_FB_MAX_ATTACHMENT - 1; type >= 0; --type) {
      GPUAttachment &attach = m_attachments[type];

      switch (type) {
        case GPU_FB_DEPTH_ATTACHMENT:
        case GPU_FB_DEPTH_STENCIL_ATTACHMENT: {
          /* If one of the DEPTH types has added a texture, we avoid running this again, as it
           * would only remove the target. */
          if (depth_added) {
            break;
          }
          if (attach.tex) {
            /* If we already had a depth attachment, preserve load/clear-state parameters,
             * but remove existing and add new attachment. */
            if (this->has_depth_attachment()) {
              MTLAttachment depth_attachment_prev = this->get_depth_attachment();
              this->remove_depth_attachment();
              this->add_depth_attachment(static_cast<gpu::MTLTexture *>(unwrap(attach.tex)),
                                         attach.mip,
                                         attach.layer);
              this->set_depth_attachment_clear_value(depth_attachment_prev.clear_value.depth);
              this->set_depth_loadstore_op(depth_attachment_prev.load_action,
                                           depth_attachment_prev.store_action);
            } else {
              this->add_depth_attachment(static_cast<gpu::MTLTexture *>(unwrap(attach.tex)),
                                         attach.mip,
                                         attach.layer);
            }

            /* Check stencil component -- if supplied texture format supports stencil. */
            eGPUTextureFormat format = GPU_texture_format(attach.tex);
            bool use_stencil = (type == GPU_FB_DEPTH_STENCIL_ATTACHMENT) &&
                               (format == GPU_DEPTH32F_STENCIL8 || format == GPU_DEPTH24_STENCIL8);
            if (use_stencil) {
              if (this->has_stencil_attachment()) {
                MTLAttachment stencil_attachment_prev = this->get_stencil_attachment();
                this->remove_stencil_attachment();
                this->add_stencil_attachment(static_cast<gpu::MTLTexture *>(unwrap(attach.tex)),
                                             attach.mip,
                                             attach.layer);
                this->set_stencil_attachment_clear_value(
                  stencil_attachment_prev.clear_value.stencil);
                this->set_stencil_loadstore_op(stencil_attachment_prev.load_action,
                                               stencil_attachment_prev.store_action);
              } else {
                this->add_stencil_attachment(static_cast<gpu::MTLTexture *>(unwrap(attach.tex)),
                                             attach.mip,
                                             attach.layer);
              }
            }

            /* Flag depth as added -- mirrors the behavior in gl_framebuffer.cc to exit the
             * for-loop after GPU_FB_DEPTH_STENCIL_ATTACHMENT has executed. */
            depth_added = true;

            if (first_attachment == GPU_FB_MAX_ATTACHMENT) {
              /* Only use depth texture to get information if there is no color attachment. */
              first_attachment = type;
              first_attachment_mtl = this->get_depth_attachment();
            }
          } else {
            this->remove_depth_attachment();
            if (type == GPU_FB_DEPTH_STENCIL_ATTACHMENT && this->has_stencil_attachment()) {
              this->remove_stencil_attachment();
            }
          }
        } break;
        case GPU_FB_COLOR_ATTACHMENT0:
        case GPU_FB_COLOR_ATTACHMENT1:
        case GPU_FB_COLOR_ATTACHMENT2:
        case GPU_FB_COLOR_ATTACHMENT3:
        case GPU_FB_COLOR_ATTACHMENT4:
        case GPU_FB_COLOR_ATTACHMENT5: {
          int color_slot_ind = type - GPU_FB_COLOR_ATTACHMENT0;
          if (attach.tex) {
            /* If we already had a colour attachment, preserve load/clear-state parameters,
             * but remove existing and add new attachment. */
            if (this->has_attachment_at_slot(color_slot_ind)) {
              MTLAttachment color_attachment_prev = this->get_color_attachment(color_slot_ind);

              this->remove_color_attachment(color_slot_ind);
              this->add_color_attachment(static_cast<gpu::MTLTexture *>(unwrap(attach.tex)),
                                         color_slot_ind,
                                         attach.mip,
                                         attach.layer);
              this->set_color_attachment_clear_color(color_slot_ind,
                                                     color_attachment_prev.clear_value.color);
              this->set_color_loadstore_op(color_slot_ind,
                                           color_attachment_prev.load_action,
                                           color_attachment_prev.store_action);
            } else {
              this->add_color_attachment(static_cast<gpu::MTLTexture *>(unwrap(attach.tex)),
                                         color_slot_ind,
                                         attach.mip,
                                         attach.layer);
            }
            first_attachment = type;
            first_attachment_mtl = this->get_color_attachment(color_slot_ind);
          } else {
            this->remove_color_attachment(color_slot_ind);
          }
        } break;
        default:
          /* Non-attachment parameters. */
          break;
      }
    }

    /* Check whether the first attachment is SRGB. */
    if (first_attachment != GPU_FB_MAX_ATTACHMENT) {
      m_is_srgb = (first_attachment_mtl.texture->format_get() == GPU_SRGB8_A8);
    }

    /* Reset viewport and Scissor (If viewport is smaller or equal to the framebuffer size). */
    if (update_viewport && t_viewport[2] <= m_width && t_viewport[3] <= m_height) {

      this->viewport_set(t_viewport);
      this->scissor_set(t_viewport);
    } else {
      this->viewport_reset();
      this->scissor_reset();
    }

    /* We have now updated our internal structures. */
    m_dirty_attachments = false;
  }

  void MTLFrameBuffer::apply_state()
  {
    MTLContext *mtl_ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(mtl_ctx);
    if (mtl_ctx->active_fb == this) {
      if (m_dirty_state == false && m_dirty_state_ctx == mtl_ctx) {
        return;
      }

      /* Ensure viewport has been set. NOTE: This should no longer happen, but kept for safety to
       * track bugs. */
      if (m_viewport[2] == 0 || m_viewport[3] == 0) {
        MTL_LOG_WARNING(
          "Viewport had width and height of (0,0) -- Updating -- DEBUG Safety check\n");
        viewport_reset();
      }

      /* Update Context State. */
      mtl_ctx->set_viewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
      mtl_ctx->set_scissor(m_scissor[0], m_scissor[1], m_scissor[2], m_scissor[3]);
      mtl_ctx->set_scissor_enabled(m_scissor_test);

      m_dirty_state = false;
      m_dirty_state_ctx = mtl_ctx;
    } else {
      MTL_LOG_ERROR(
        "Attempting to set FrameBuffer State (VIEWPORT, SCISSOR), But FrameBuffer is not bound to "
        "current Context.\n");
    }
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @ Adding and Removing attachments
   * @{ */

  bool MTLFrameBuffer::add_color_attachment(gpu::MTLTexture *texture,
                                            uint slot,
                                            int miplevel,
                                            int layer)
  {
    KLI_assert(this);
    KLI_assert(slot >= 0 && slot < this->get_attachment_limit());

    if (texture) {
      if (miplevel < 0 || miplevel >= MTL_MAX_MIPMAP_COUNT) {
        MTL_LOG_WARNING("Attachment specified with invalid mip level %u\n", miplevel);
        miplevel = 0;
      }

      /* Check if slot is in-use. */
      /* Assume attachment load by default. */
      m_colour_attachment_count += (!m_mtl_color_attachments[slot].used) ? 1 : 0;
      m_mtl_color_attachments[slot].used = true;
      m_mtl_color_attachments[slot].texture = texture;
      m_mtl_color_attachments[slot].mip = miplevel;
      m_mtl_color_attachments[slot].load_action = GPU_LOADACTION_LOAD;
      m_mtl_color_attachments[slot].store_action = GPU_STOREACTION_STORE;
      m_mtl_color_attachments[slot].render_target_array_length = 0;

      /* Determine whether array slice or depth plane based on texture type. */
      switch (texture->m_type) {
        case GPU_TEXTURE_1D:
        case GPU_TEXTURE_2D:
          KLI_assert(layer <= 0);
          m_mtl_color_attachments[slot].slice = 0;
          m_mtl_color_attachments[slot].depth_plane = 0;
          break;
        case GPU_TEXTURE_1D_ARRAY:
          if (layer < 0) {
            layer = 0;
            MTL_LOG_WARNING("TODO: Support layered rendering for 1D array textures, if needed.\n");
          }
          KLI_assert(layer < texture->m_h);
          m_mtl_color_attachments[slot].slice = layer;
          m_mtl_color_attachments[slot].depth_plane = 0;
          break;
        case GPU_TEXTURE_2D_ARRAY:
          KLI_assert(layer < texture->m_d);
          m_mtl_color_attachments[slot].slice = layer;
          m_mtl_color_attachments[slot].depth_plane = 0;
          if (layer == -1) {
            m_mtl_color_attachments[slot].slice = 0;
            m_mtl_color_attachments[slot].render_target_array_length = texture->m_d;
            m_use_multilayered_rendering = true;
          }
          break;
        case GPU_TEXTURE_3D:
          KLI_assert(layer < texture->m_d);
          m_mtl_color_attachments[slot].slice = 0;
          m_mtl_color_attachments[slot].depth_plane = layer;
          if (layer == -1) {
            m_mtl_color_attachments[slot].depth_plane = 0;
            m_mtl_color_attachments[slot].render_target_array_length = texture->m_d;
            m_use_multilayered_rendering = true;
          }
          break;
        case GPU_TEXTURE_CUBE:
          KLI_assert(layer < 6);
          m_mtl_color_attachments[slot].slice = layer;
          m_mtl_color_attachments[slot].depth_plane = 0;
          if (layer == -1) {
            m_mtl_color_attachments[slot].slice = 0;
            m_mtl_color_attachments[slot].depth_plane = 0;
            m_mtl_color_attachments[slot].render_target_array_length = 6;
            m_use_multilayered_rendering = true;
          }
          break;
        case GPU_TEXTURE_CUBE_ARRAY:
          KLI_assert(layer < 6 * texture->m_d);
          /* TODO(Metal): Verify multilayered rendering for Cube arrays. */
          m_mtl_color_attachments[slot].slice = layer;
          m_mtl_color_attachments[slot].depth_plane = 0;
          if (layer == -1) {
            m_mtl_color_attachments[slot].slice = 0;
            m_mtl_color_attachments[slot].depth_plane = 0;
            m_mtl_color_attachments[slot].render_target_array_length = texture->m_d;
            m_use_multilayered_rendering = true;
          }
          break;
        case GPU_TEXTURE_BUFFER:
          m_mtl_color_attachments[slot].slice = 0;
          m_mtl_color_attachments[slot].depth_plane = 0;
          break;
        default:
          MTL_LOG_ERROR("MTLFrameBuffer::add_color_attachment Unrecognized texture type %u\n",
                        texture->m_type);
          break;
      }

      /* Update Frame-buffer Resolution. */
      int width_of_miplayer, height_of_miplayer;
      if (miplevel <= 0) {
        width_of_miplayer = texture->width_get();
        height_of_miplayer = texture->height_get();
      } else {
        width_of_miplayer = max_ii(texture->width_get() >> miplevel, 1);
        height_of_miplayer = max_ii(texture->height_get() >> miplevel, 1);
      }

      if (m_width == 0 || m_height == 0) {
        this->size_set(width_of_miplayer, height_of_miplayer);
        this->scissor_reset();
        this->viewport_reset();
        KLI_assert(m_width > 0);
        KLI_assert(m_height > 0);
      } else {
        KLI_assert(m_width == width_of_miplayer);
        KLI_assert(m_height == height_of_miplayer);
      }

      /* Flag as dirty. */
      this->mark_dirty();
    } else {
      MTL_LOG_ERROR(
        "Passing in null texture to MTLFrameBuffer::addColourAttachment (This could be due to not "
        "all texture types being supported).\n");
    }
    return true;
  }

  bool MTLFrameBuffer::add_depth_attachment(gpu::MTLTexture *texture, int miplevel, int layer)
  {
    KLI_assert(this);

    if (texture) {
      if (miplevel < 0 || miplevel >= MTL_MAX_MIPMAP_COUNT) {
        MTL_LOG_WARNING("Attachment specified with invalid mip level %u\n", miplevel);
        miplevel = 0;
      }

      /* Assume attachment load by default. */
      m_mtl_depth_attachment.used = true;
      m_mtl_depth_attachment.texture = texture;
      m_mtl_depth_attachment.mip = miplevel;
      m_mtl_depth_attachment.load_action = GPU_LOADACTION_LOAD;
      m_mtl_depth_attachment.store_action = GPU_STOREACTION_STORE;
      m_mtl_depth_attachment.render_target_array_length = 0;

      /* Determine whether array slice or depth plane based on texture type. */
      switch (texture->m_type) {
        case GPU_TEXTURE_1D:
        case GPU_TEXTURE_2D:
          KLI_assert(layer <= 0);
          m_mtl_depth_attachment.slice = 0;
          m_mtl_depth_attachment.depth_plane = 0;
          break;
        case GPU_TEXTURE_1D_ARRAY:
          if (layer < 0) {
            layer = 0;
            MTL_LOG_WARNING("TODO: Support layered rendering for 1D array textures, if needed\n");
          }
          KLI_assert(layer < texture->m_h);
          m_mtl_depth_attachment.slice = layer;
          m_mtl_depth_attachment.depth_plane = 0;
          break;
        case GPU_TEXTURE_2D_ARRAY:
          KLI_assert(layer < texture->m_d);
          m_mtl_depth_attachment.slice = layer;
          m_mtl_depth_attachment.depth_plane = 0;
          if (layer == -1) {
            m_mtl_depth_attachment.slice = 0;
            m_mtl_depth_attachment.render_target_array_length = texture->m_d;
            m_use_multilayered_rendering = true;
          }
          break;
        case GPU_TEXTURE_3D:
          KLI_assert(layer < texture->m_d);
          m_mtl_depth_attachment.slice = 0;
          m_mtl_depth_attachment.depth_plane = layer;
          if (layer == -1) {
            m_mtl_depth_attachment.depth_plane = 0;
            m_mtl_depth_attachment.render_target_array_length = texture->m_d;
            m_use_multilayered_rendering = true;
          }
          break;
        case GPU_TEXTURE_CUBE:
          KLI_assert(layer < 6);
          m_mtl_depth_attachment.slice = layer;
          m_mtl_depth_attachment.depth_plane = 0;
          if (layer == -1) {
            m_mtl_depth_attachment.slice = 0;
            m_mtl_depth_attachment.depth_plane = 0;
            m_mtl_depth_attachment.render_target_array_length = 1;
            m_use_multilayered_rendering = true;
          }
          break;
        case GPU_TEXTURE_CUBE_ARRAY:
          /* TODO(Metal): Verify multilayered rendering for Cube arrays. */
          KLI_assert(layer < 6 * texture->m_d);
          m_mtl_depth_attachment.slice = layer;
          m_mtl_depth_attachment.depth_plane = 0;
          if (layer == -1) {
            m_mtl_depth_attachment.slice = 0;
            m_mtl_depth_attachment.depth_plane = 0;
            m_mtl_depth_attachment.render_target_array_length = texture->m_d;
            m_use_multilayered_rendering = true;
          }
          break;
        case GPU_TEXTURE_BUFFER:
          m_mtl_depth_attachment.slice = 0;
          m_mtl_depth_attachment.depth_plane = 0;
          break;
        default:
          KLI_assert(false && "Unrecognized texture type");
          break;
      }

      /* Update Frame-buffer Resolution. */
      int width_of_miplayer, height_of_miplayer;
      if (miplevel <= 0) {
        width_of_miplayer = texture->width_get();
        height_of_miplayer = texture->height_get();
      } else {
        width_of_miplayer = max_ii(texture->width_get() >> miplevel, 1);
        height_of_miplayer = max_ii(texture->height_get() >> miplevel, 1);
      }

      /* Update Frame-buffer Resolution. */
      if (m_width == 0 || m_height == 0) {
        this->size_set(width_of_miplayer, height_of_miplayer);
        this->scissor_reset();
        this->viewport_reset();
        KLI_assert(m_width > 0);
        KLI_assert(m_height > 0);
      } else {
        KLI_assert(m_width == texture->width_get());
        KLI_assert(m_height == texture->height_get());
      }

      /* Flag as dirty after attachments changed. */
      this->mark_dirty();
    } else {
      MTL_LOG_ERROR(
        "Passing in null texture to MTLFrameBuffer::addDepthAttachment (This could be due to not "
        "all texture types being supported).");
    }
    return true;
  }

  bool MTLFrameBuffer::add_stencil_attachment(gpu::MTLTexture *texture, int miplevel, int layer)
  {
    KLI_assert(this);

    if (texture) {
      if (miplevel < 0 || miplevel >= MTL_MAX_MIPMAP_COUNT) {
        MTL_LOG_WARNING("Attachment specified with invalid mip level %u\n", miplevel);
        miplevel = 0;
      }

      /* Assume attachment load by default. */
      m_mtl_stencil_attachment.used = true;
      m_mtl_stencil_attachment.texture = texture;
      m_mtl_stencil_attachment.mip = miplevel;
      m_mtl_stencil_attachment.load_action = GPU_LOADACTION_LOAD;
      m_mtl_stencil_attachment.store_action = GPU_STOREACTION_STORE;
      m_mtl_stencil_attachment.render_target_array_length = 0;

      /* Determine whether array slice or depth plane based on texture type. */
      switch (texture->m_type) {
        case GPU_TEXTURE_1D:
        case GPU_TEXTURE_2D:
          KLI_assert(layer <= 0);
          m_mtl_stencil_attachment.slice = 0;
          m_mtl_stencil_attachment.depth_plane = 0;
          break;
        case GPU_TEXTURE_1D_ARRAY:
          if (layer < 0) {
            layer = 0;
            MTL_LOG_WARNING("TODO: Support layered rendering for 1D array textures, if needed\n");
          }
          KLI_assert(layer < texture->m_h);
          m_mtl_stencil_attachment.slice = layer;
          m_mtl_stencil_attachment.depth_plane = 0;
          break;
        case GPU_TEXTURE_2D_ARRAY:
          KLI_assert(layer < texture->m_d);
          m_mtl_stencil_attachment.slice = layer;
          m_mtl_stencil_attachment.depth_plane = 0;
          if (layer == -1) {
            m_mtl_stencil_attachment.slice = 0;
            m_mtl_stencil_attachment.render_target_array_length = texture->m_d;
            m_use_multilayered_rendering = true;
          }
          break;
        case GPU_TEXTURE_3D:
          KLI_assert(layer < texture->m_d);
          m_mtl_stencil_attachment.slice = 0;
          m_mtl_stencil_attachment.depth_plane = layer;
          if (layer == -1) {
            m_mtl_stencil_attachment.depth_plane = 0;
            m_mtl_stencil_attachment.render_target_array_length = texture->m_d;
            m_use_multilayered_rendering = true;
          }
          break;
        case GPU_TEXTURE_CUBE:
          KLI_assert(layer < 6);
          m_mtl_stencil_attachment.slice = layer;
          m_mtl_stencil_attachment.depth_plane = 0;
          if (layer == -1) {
            m_mtl_stencil_attachment.slice = 0;
            m_mtl_stencil_attachment.depth_plane = 0;
            m_mtl_stencil_attachment.render_target_array_length = 1;
            m_use_multilayered_rendering = true;
          }
          break;
        case GPU_TEXTURE_CUBE_ARRAY:
          /* TODO(Metal): Verify multilayered rendering for Cube arrays. */
          KLI_assert(layer < 6 * texture->m_d);
          m_mtl_stencil_attachment.slice = layer;
          m_mtl_stencil_attachment.depth_plane = 0;
          if (layer == -1) {
            m_mtl_stencil_attachment.slice = 0;
            m_mtl_stencil_attachment.depth_plane = 0;
            m_mtl_stencil_attachment.render_target_array_length = texture->m_d;
            m_use_multilayered_rendering = true;
          }
          break;
        case GPU_TEXTURE_BUFFER:
          m_mtl_stencil_attachment.slice = 0;
          m_mtl_stencil_attachment.depth_plane = 0;
          break;
        default:
          KLI_assert(false && "Unrecognized texture type");
          break;
      }

      /* Update Frame-buffer Resolution. */
      int width_of_miplayer, height_of_miplayer;
      if (miplevel <= 0) {
        width_of_miplayer = texture->width_get();
        height_of_miplayer = texture->height_get();
      } else {
        width_of_miplayer = max_ii(texture->width_get() >> miplevel, 1);
        height_of_miplayer = max_ii(texture->height_get() >> miplevel, 1);
      }

      /* Update Frame-buffer Resolution. */
      if (m_width == 0 || m_height == 0) {
        this->size_set(width_of_miplayer, height_of_miplayer);
        this->scissor_reset();
        this->viewport_reset();
        KLI_assert(m_width > 0);
        KLI_assert(m_height > 0);
      } else {
        KLI_assert(m_width == texture->width_get());
        KLI_assert(m_height == texture->height_get());
      }

      /* Flag as dirty after attachments changed. */
      this->mark_dirty();
    } else {
      MTL_LOG_ERROR(
        "Passing in null texture to MTLFrameBuffer::addStencilAttachment (This could be due to "
        "not all texture types being supported).");
    }
    return true;
  }

  bool MTLFrameBuffer::remove_color_attachment(uint slot)
  {
    KLI_assert(this);
    KLI_assert(slot >= 0 && slot < this->get_attachment_limit());

    if (this->has_attachment_at_slot(slot)) {
      m_colour_attachment_count -= (m_mtl_color_attachments[slot].used) ? 1 : 0;
      m_mtl_color_attachments[slot].used = false;
      this->ensure_render_target_size();
      this->mark_dirty();
      return true;
    }

    return false;
  }

  bool MTLFrameBuffer::remove_depth_attachment()
  {
    KLI_assert(this);

    m_mtl_depth_attachment.used = false;
    m_mtl_depth_attachment.texture = nullptr;
    this->ensure_render_target_size();
    this->mark_dirty();

    return true;
  }

  bool MTLFrameBuffer::remove_stencil_attachment()
  {
    KLI_assert(this);

    m_mtl_stencil_attachment.used = false;
    m_mtl_stencil_attachment.texture = nullptr;
    this->ensure_render_target_size();
    this->mark_dirty();

    return true;
  }

  void MTLFrameBuffer::remove_all_attachments()
  {
    KLI_assert(this);

    for (int attachment = 0; attachment < GPU_FB_MAX_COLOR_ATTACHMENT; attachment++) {
      this->remove_color_attachment(attachment);
    }
    this->remove_depth_attachment();
    this->remove_stencil_attachment();
    m_colour_attachment_count = 0;
    this->mark_dirty();

    /* Verify height. */
    this->ensure_render_target_size();

    /* Flag attachments as no longer being dirty. */
    m_dirty_attachments = false;
  }

  void MTLFrameBuffer::ensure_render_target_size()
  {
    /* If we have no attachments, reset width and height to zero. */
    if (m_colour_attachment_count == 0 && !this->has_depth_attachment() &&
        !this->has_stencil_attachment()) {

      /* Reset Viewport and Scissor for NULL framebuffer. */
      this->size_set(0, 0);
      this->scissor_reset();
      this->viewport_reset();
    }
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @ Clear values and Load-store actions
   * @{ */

  void MTLFrameBuffer::attachment_set_loadstore_op(GPUAttachmentType type,
                                                   eGPULoadOp load_action,
                                                   eGPUStoreOp store_action)
  {
    if (type >= GPU_FB_COLOR_ATTACHMENT0) {
      int slot = type - GPU_FB_COLOR_ATTACHMENT0;
      this->set_color_loadstore_op(slot, load_action, store_action);
    } else if (type == GPU_FB_DEPTH_STENCIL_ATTACHMENT) {
      this->set_depth_loadstore_op(load_action, store_action);
      this->set_stencil_loadstore_op(load_action, store_action);
    } else if (type == GPU_FB_DEPTH_ATTACHMENT) {
      this->set_depth_loadstore_op(load_action, store_action);
    }
  }

  bool MTLFrameBuffer::set_color_attachment_clear_color(uint slot, const float clear_color[4])
  {
    KLI_assert(this);
    KLI_assert(slot >= 0 && slot < this->get_attachment_limit());

    /* Only mark as dirty if values have changed. */
    bool changed = m_mtl_color_attachments[slot].load_action != GPU_LOADACTION_CLEAR;
    changed = changed || (memcmp(m_mtl_color_attachments[slot].clear_value.color,
                                 clear_color,
                                 sizeof(float) * 4) != 0);
    if (changed) {
      memcpy(m_mtl_color_attachments[slot].clear_value.color, clear_color, sizeof(float) * 4);
    }
    m_mtl_color_attachments[slot].load_action = GPU_LOADACTION_CLEAR;

    if (changed) {
      this->mark_loadstore_dirty();
    }
    return true;
  }

  bool MTLFrameBuffer::set_depth_attachment_clear_value(float depth_clear)
  {
    KLI_assert(this);

    if (m_mtl_depth_attachment.clear_value.depth != depth_clear ||
        m_mtl_depth_attachment.load_action != GPU_LOADACTION_CLEAR) {
      m_mtl_depth_attachment.clear_value.depth = depth_clear;
      m_mtl_depth_attachment.load_action = GPU_LOADACTION_CLEAR;
      this->mark_loadstore_dirty();
    }
    return true;
  }

  bool MTLFrameBuffer::set_stencil_attachment_clear_value(uint stencil_clear)
  {
    KLI_assert(this);

    if (m_mtl_stencil_attachment.clear_value.stencil != stencil_clear ||
        m_mtl_stencil_attachment.load_action != GPU_LOADACTION_CLEAR) {
      m_mtl_stencil_attachment.clear_value.stencil = stencil_clear;
      m_mtl_stencil_attachment.load_action = GPU_LOADACTION_CLEAR;
      this->mark_loadstore_dirty();
    }
    return true;
  }

  bool MTLFrameBuffer::set_color_loadstore_op(uint slot,
                                              eGPULoadOp load_action,
                                              eGPUStoreOp store_action)
  {
    KLI_assert(this);
    eGPULoadOp prev_load_action = m_mtl_color_attachments[slot].load_action;
    eGPUStoreOp prev_store_action = m_mtl_color_attachments[slot].store_action;
    m_mtl_color_attachments[slot].load_action = load_action;
    m_mtl_color_attachments[slot].store_action = store_action;

    bool changed = (m_mtl_color_attachments[slot].load_action != prev_load_action ||
                    m_mtl_color_attachments[slot].store_action != prev_store_action);
    if (changed) {
      this->mark_loadstore_dirty();
    }

    return changed;
  }

  bool MTLFrameBuffer::set_depth_loadstore_op(eGPULoadOp load_action, eGPUStoreOp store_action)
  {
    KLI_assert(this);
    eGPULoadOp prev_load_action = m_mtl_depth_attachment.load_action;
    eGPUStoreOp prev_store_action = m_mtl_depth_attachment.store_action;
    m_mtl_depth_attachment.load_action = load_action;
    m_mtl_depth_attachment.store_action = store_action;

    bool changed = (m_mtl_depth_attachment.load_action != prev_load_action ||
                    m_mtl_depth_attachment.store_action != prev_store_action);
    if (changed) {
      this->mark_loadstore_dirty();
    }

    return changed;
  }

  bool MTLFrameBuffer::set_stencil_loadstore_op(eGPULoadOp load_action, eGPUStoreOp store_action)
  {
    KLI_assert(this);
    eGPULoadOp prev_load_action = m_mtl_stencil_attachment.load_action;
    eGPUStoreOp prev_store_action = m_mtl_stencil_attachment.store_action;
    m_mtl_stencil_attachment.load_action = load_action;
    m_mtl_stencil_attachment.store_action = store_action;

    bool changed = (m_mtl_stencil_attachment.load_action != prev_load_action ||
                    m_mtl_stencil_attachment.store_action != prev_store_action);
    if (changed) {
      this->mark_loadstore_dirty();
    }

    return changed;
  }

  bool MTLFrameBuffer::reset_clear_state()
  {
    for (int slot = 0; slot < m_colour_attachment_count; slot++) {
      this->set_color_loadstore_op(slot, GPU_LOADACTION_LOAD, GPU_STOREACTION_STORE);
    }
    this->set_depth_loadstore_op(GPU_LOADACTION_LOAD, GPU_STOREACTION_STORE);
    this->set_stencil_loadstore_op(GPU_LOADACTION_LOAD, GPU_STOREACTION_STORE);
    return true;
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @ Fetch values and Frame-buffer status
   * @{ */

  bool MTLFrameBuffer::has_attachment_at_slot(uint slot)
  {
    KLI_assert(this);

    if (slot >= 0 && slot < this->get_attachment_limit()) {
      return m_mtl_color_attachments[slot].used;
    }
    return false;
  }

  bool MTLFrameBuffer::has_color_attachment_with_texture(gpu::MTLTexture *texture)
  {
    KLI_assert(this);

    for (int attachment = 0; attachment < this->get_attachment_limit(); attachment++) {
      if (m_mtl_color_attachments[attachment].used &&
          m_mtl_color_attachments[attachment].texture == texture) {
        return true;
      }
    }
    return false;
  }

  bool MTLFrameBuffer::has_depth_attachment()
  {
    KLI_assert(this);
    return m_mtl_depth_attachment.used;
  }

  bool MTLFrameBuffer::has_stencil_attachment()
  {
    KLI_assert(this);
    return m_mtl_stencil_attachment.used;
  }

  int MTLFrameBuffer::get_color_attachment_slot_from_texture(gpu::MTLTexture *texture)
  {
    KLI_assert(this);
    KLI_assert(texture);

    for (int attachment = 0; attachment < this->get_attachment_limit(); attachment++) {
      if (m_mtl_color_attachments[attachment].used &&
          (m_mtl_color_attachments[attachment].texture == texture)) {
        return attachment;
      }
    }
    return -1;
  }

  uint MTLFrameBuffer::get_attachment_count()
  {
    KLI_assert(this);
    return m_colour_attachment_count;
  }

  MTLAttachment MTLFrameBuffer::get_color_attachment(uint slot)
  {
    KLI_assert(this);
    if (slot >= 0 && slot < GPU_FB_MAX_COLOR_ATTACHMENT) {
      return m_mtl_color_attachments[slot];
    }
    MTLAttachment null_attachment;
    null_attachment.used = false;
    return null_attachment;
  }

  MTLAttachment MTLFrameBuffer::get_depth_attachment()
  {
    KLI_assert(this);
    return m_mtl_depth_attachment;
  }

  MTLAttachment MTLFrameBuffer::get_stencil_attachment()
  {
    KLI_assert(this);
    return m_mtl_stencil_attachment;
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @ METAL API Resources and Validation
   * @{ */
  bool MTLFrameBuffer::validate_render_pass()
  {
    KLI_assert(this);

    /* First update attachments if dirty. */
    this->update_attachments(true);

    /* Verify attachment count. */
    int used_attachments = 0;
    for (int attachment = 0; attachment < GPU_FB_MAX_COLOR_ATTACHMENT; attachment++) {
      if (m_mtl_color_attachments[attachment].used) {
        used_attachments++;
      }
    }
    used_attachments += (m_mtl_depth_attachment.used) ? 1 : 0;
    used_attachments += (m_mtl_stencil_attachment.used) ? 1 : 0;
    return (used_attachments > 0);
  }

  MTL::LoadAction mtl_load_action_from_gpu(eGPULoadOp action)
  {
    return (action == GPU_LOADACTION_LOAD) ?
             MTL::LoadActionLoad :
             ((action == GPU_LOADACTION_CLEAR) ? MTL::LoadActionClear : MTL::LoadActionDontCare);
  }

  MTL::StoreAction mtl_store_action_from_gpu(eGPUStoreOp action)
  {
    return (action == GPU_STOREACTION_STORE) ? MTL::StoreActionStore : MTL::StoreActionDontCare;
  }

  MTL::RenderPassDescriptor *MTLFrameBuffer::bake_render_pass_descriptor(bool load_contents)
  {
    KLI_assert(this);
    if (load_contents) {
      /* Only force-load contents if there is no clear pending. */
      KLI_assert(!m_has_pending_clear);
    }

    /* Ensure we are inside a frame boundary. */
    MTLContext *metal_ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(metal_ctx && metal_ctx->get_inside_frame());
    UNUSED_VARS_NDEBUG(metal_ctx);

    /* If Frame-buffer has been modified, regenerate descriptor. */
    if (m_is_dirty) {
      /* Clear all configs. */
      for (int config = 0; config < 3; config++) {
        m_descriptor_dirty[config] = true;
      }
    } else if (m_is_loadstore_dirty) {
      /* Load config always has load ops, so we only need to re-generate custom and clear state. */
      m_descriptor_dirty[MTL_FB_CONFIG_CLEAR] = true;
      m_descriptor_dirty[MTL_FB_CONFIG_CUSTOM] = true;
    }

    /* If we need to populate descriptor" */
    /* Select config based on FrameBuffer state:
     * [0] {MTL_FB_CONFIG_CLEAR} = Clear config -- we have a pending clear so should perform our
     * configured clear.
     * [1] {MTL_FB_CONFIG_LOAD} = Load config -- We need to re-load ALL attachments,
     * used for re-binding/pass-breaks.
     * [2] {MTL_FB_CONFIG_CUSTOM} = Custom config -- Use this when a custom binding config is
     * specified.
     */
    uint descriptor_config = (load_contents) ?
                               MTL_FB_CONFIG_LOAD :
                               ((this->get_pending_clear()) ? MTL_FB_CONFIG_CLEAR :
                                                              MTL_FB_CONFIG_CUSTOM);
    if (m_descriptor_dirty[descriptor_config] ||
        m_framebuffer_descriptor[descriptor_config] == nil) {

      /* Create descriptor if it does not exist. */
      if (m_framebuffer_descriptor[descriptor_config] == nil) {
        m_framebuffer_descriptor[descriptor_config] = MTL::RenderPassDescriptor::alloc()->init();
      }

#if defined(MAC_OS_VERSION_11_0) && __MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_VERSION_11_0
      /* Optimization: Use smaller tile size on Apple Silicon if exceeding a certain bpp limit. */
      bool is_tile_based_gpu = metal_ctx->device->hasUnifiedMemory();
      if (is_tile_based_gpu) {
        uint framebuffer_bpp = this->get_bits_per_pixel();
        bool use_small_tiles = (framebuffer_bpp > 64);

        if (use_small_tiles) {
          m_framebuffer_descriptor[descriptor_config]->setTileWidth(16);
          m_framebuffer_descriptor[descriptor_config]->setTileHeight(16);
        }
      }
#endif

      /* Configure multilayered rendering. */
      if (m_use_multilayered_rendering) {
        /* Ensure all targets have the same length. */
        int len = 0;
        bool valid = true;

        for (int attachment_ind = 0; attachment_ind < GPU_FB_MAX_COLOR_ATTACHMENT;
             attachment_ind++) {
          if (m_mtl_color_attachments[attachment_ind].used) {
            if (len == 0) {
              len = m_mtl_color_attachments[attachment_ind].render_target_array_length;
            } else {
              valid = valid &&
                      (len == m_mtl_color_attachments[attachment_ind].render_target_array_length);
            }
          }
        }

        if (m_mtl_depth_attachment.used) {
          if (len == 0) {
            len = m_mtl_depth_attachment.render_target_array_length;
          } else {
            valid = valid && (len == m_mtl_depth_attachment.render_target_array_length);
          }
        }

        if (m_mtl_stencil_attachment.used) {
          if (len == 0) {
            len = m_mtl_stencil_attachment.render_target_array_length;
          } else {
            valid = valid && (len == m_mtl_stencil_attachment.render_target_array_length);
          }
        }

        KLI_assert(len > 0);
        KLI_assert(valid);
        m_framebuffer_descriptor[descriptor_config]->setRenderTargetArrayLength(len);
      } else {
        m_framebuffer_descriptor[descriptor_config]->setRenderTargetArrayLength(0);
      }

      /* Color attachments. */
      int colour_attachments = 0;
      for (int attachment_ind = 0; attachment_ind < GPU_FB_MAX_COLOR_ATTACHMENT;
           attachment_ind++) {

        if (m_mtl_color_attachments[attachment_ind].used) {

          /* Create attachment descriptor. */
          MTL::RenderPassColorAttachmentDescriptor *attachment =
            m_colour_attachment_descriptors[attachment_ind];
          KLI_assert(attachment != nil);

          MTL::Texture *texture =
            m_mtl_color_attachments[attachment_ind].texture->get_metal_handle_base();
          if (texture == nil) {
            MTL_LOG_ERROR("Attempting to assign invalid texture as attachment\n");
          }

          /* IF SRGB is enabled, but we are rendering with SRGB disabled, sample texture view. */
          /* TODO(Metal): Consider caching SRGB texture view. */
          MTL::Texture *source_color_texture = texture;
          if (this->get_is_srgb() && !this->get_srgb_enabled()) {
            source_color_texture = texture->newTextureView(MTL::PixelFormatRGBA8Unorm);
          }

          /* Resolve appropriate load action -- IF force load, perform load.
           * If clear but framebuffer has no pending clear, also load. */
          eGPULoadOp load_action = m_mtl_color_attachments[attachment_ind].load_action;
          if (descriptor_config == MTL_FB_CONFIG_LOAD) {
            /* MTL_FB_CONFIG_LOAD must always load. */
            load_action = GPU_LOADACTION_LOAD;
          } else if (descriptor_config == MTL_FB_CONFIG_CUSTOM &&
                     load_action == GPU_LOADACTION_CLEAR) {
            /* Custom config should be LOAD or DONT_CARE only. */
            load_action = GPU_LOADACTION_LOAD;
          }
          attachment->setTexture(source_color_texture);
          attachment->setLoadAction(mtl_load_action_from_gpu(load_action));
          attachment->setClearColor(
            (load_action == GPU_LOADACTION_CLEAR) ?
              MTL::ClearColor::Make(m_mtl_color_attachments[attachment_ind].clear_value.color[0],
                                    m_mtl_color_attachments[attachment_ind].clear_value.color[1],
                                    m_mtl_color_attachments[attachment_ind].clear_value.color[2],
                                    m_mtl_color_attachments[attachment_ind].clear_value.color[3]) :
              MTL::ClearColor::Make(0.0, 0.0, 0.0, 0.0));
          attachment->setStoreAction(
            mtl_store_action_from_gpu(m_mtl_color_attachments[attachment_ind].store_action));
          attachment->setLevel(m_mtl_color_attachments[attachment_ind].mip);
          attachment->setSlice(m_mtl_color_attachments[attachment_ind].slice);
          attachment->setDepthPlane(m_mtl_color_attachments[attachment_ind].depth_plane);
          colour_attachments++;

          /* Copy attachment info back in. */
          m_framebuffer_descriptor[descriptor_config]->colorAttachments()->setObject(
            attachment,
            attachment_ind);

        } else {
          /* Disable colour attachment. */
          m_framebuffer_descriptor[descriptor_config]->colorAttachments()->setObject(
            nil,
            attachment_ind);
        }
      }
      KLI_assert(colour_attachments == m_colour_attachment_count);

      /* Depth attachment. */
      if (m_mtl_depth_attachment.used) {
        m_framebuffer_descriptor[descriptor_config]->depthAttachment()->setTexture(
          m_mtl_depth_attachment.texture->get_metal_handle_base());

        /* Resolve appropriate load action -- IF force load, perform load.
         * If clear but framebuffer has no pending clear, also load. */
        eGPULoadOp load_action = m_mtl_depth_attachment.load_action;
        if (descriptor_config == MTL_FB_CONFIG_LOAD) {
          /* MTL_FB_CONFIG_LOAD must always load. */
          load_action = GPU_LOADACTION_LOAD;
        } else if (descriptor_config == MTL_FB_CONFIG_CUSTOM &&
                   load_action == GPU_LOADACTION_CLEAR) {
          /* Custom config should be LOAD or DONT_CARE only. */
          load_action = GPU_LOADACTION_LOAD;
        }
        m_framebuffer_descriptor[descriptor_config]->depthAttachment()->setLoadAction(
          mtl_load_action_from_gpu(load_action));
        m_framebuffer_descriptor[descriptor_config]->depthAttachment()->setClearDepth(
          (load_action == GPU_LOADACTION_CLEAR) ? m_mtl_depth_attachment.clear_value.depth : 0);
        m_framebuffer_descriptor[descriptor_config]->depthAttachment()->setStoreAction(
          mtl_store_action_from_gpu(m_mtl_depth_attachment.store_action));
        m_framebuffer_descriptor[descriptor_config]->depthAttachment()->setLevel(
          m_mtl_depth_attachment.mip);
        m_framebuffer_descriptor[descriptor_config]->depthAttachment()->setSlice(
          m_mtl_depth_attachment.slice);
        m_framebuffer_descriptor[descriptor_config]->depthAttachment()->setDepthPlane(
          m_mtl_depth_attachment.depth_plane);
      } else {
        m_framebuffer_descriptor[descriptor_config]->depthAttachment()->setTexture(nil);
      }

      /*  Stencil attachment. */
      if (m_mtl_stencil_attachment.used) {
        m_framebuffer_descriptor[descriptor_config]->stencilAttachment()->setTexture(
          m_mtl_stencil_attachment.texture->get_metal_handle_base());

        /* Resolve appropriate load action -- IF force load, perform load.
         * If clear but framebuffer has no pending clear, also load. */
        eGPULoadOp load_action = m_mtl_stencil_attachment.load_action;
        if (descriptor_config == MTL_FB_CONFIG_LOAD) {
          /* MTL_FB_CONFIG_LOAD must always load. */
          load_action = GPU_LOADACTION_LOAD;
        } else if (descriptor_config == MTL_FB_CONFIG_CUSTOM &&
                   load_action == GPU_LOADACTION_CLEAR) {
          /* Custom config should be LOAD or DONT_CARE only. */
          load_action = GPU_LOADACTION_LOAD;
        }
        m_framebuffer_descriptor[descriptor_config]->stencilAttachment()->setLoadAction(
          mtl_load_action_from_gpu(load_action));
        m_framebuffer_descriptor[descriptor_config]->stencilAttachment()->setClearStencil(
          (load_action == GPU_LOADACTION_CLEAR) ? m_mtl_stencil_attachment.clear_value.stencil :
                                                  0);
        m_framebuffer_descriptor[descriptor_config]->stencilAttachment()->setStoreAction(
          mtl_store_action_from_gpu(m_mtl_stencil_attachment.store_action));
        m_framebuffer_descriptor[descriptor_config]->stencilAttachment()->setLevel(
          m_mtl_stencil_attachment.mip);
        m_framebuffer_descriptor[descriptor_config]->stencilAttachment()->setSlice(
          m_mtl_stencil_attachment.slice);
        m_framebuffer_descriptor[descriptor_config]->stencilAttachment()->setDepthPlane(
          m_mtl_stencil_attachment.depth_plane);
      } else {
        m_framebuffer_descriptor[descriptor_config]->stencilAttachment()->setTexture(nil);
      }
      m_descriptor_dirty[descriptor_config] = false;
    }
    m_is_dirty = false;
    m_is_loadstore_dirty = false;
    return m_framebuffer_descriptor[descriptor_config];
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @ Blitting
   * @{ */

  void MTLFrameBuffer::blit(uint read_slot,
                            uint src_x_offset,
                            uint src_y_offset,
                            MTLFrameBuffer *metal_fb_write,
                            uint write_slot,
                            uint dst_x_offset,
                            uint dst_y_offset,
                            uint width,
                            uint height,
                            eGPUFrameBufferBits blit_buffers)
  {
    KLI_assert(this);
    KLI_assert(metal_fb_write);
    if (!(this && metal_fb_write)) {
      return;
    }
    MTLContext *mtl_context = reinterpret_cast<MTLContext *>(GPU_context_active_get());

    const bool do_color = (blit_buffers & GPU_COLOR_BIT);
    const bool do_depth = (blit_buffers & GPU_DEPTH_BIT);
    const bool do_stencil = (blit_buffers & GPU_STENCIL_BIT);

    /* Early exit if there is no blit to do. */
    if (!(do_color || do_depth || do_stencil)) {
      MTL_LOG_WARNING(
        " MTLFrameBuffer: requested blit but no color, depth or stencil flag was set\n");
      return;
    }

    MTL::BlitCommandEncoder *blit_encoder = nil;

    /* If the color format is not the same, we cannot use the BlitCommandEncoder, and instead use
     * a Graphics-based blit. */
    if (do_color && (this->get_color_attachment(read_slot).texture->format_get() !=
                     metal_fb_write->get_color_attachment(read_slot).texture->format_get())) {

      MTLAttachment src_attachment = this->get_color_attachment(read_slot);
      MTLAttachment dst_attachment = metal_fb_write->get_color_attachment(write_slot);
      assert(src_attachment.slice == 0 &&
             "currently only supporting slice 0 for graphics framebuffer blit");

      src_attachment.texture->blit(dst_attachment.texture,
                                   src_x_offset,
                                   src_y_offset,
                                   dst_x_offset,
                                   dst_y_offset,
                                   src_attachment.mip,
                                   dst_attachment.mip,
                                   dst_attachment.slice,
                                   width,
                                   height);
    } else {

      /* Setup blit encoder. */
      blit_encoder = mtl_context->main_command_buffer.ensure_begin_blit_encoder();

      if (do_color) {
        MTLAttachment src_attachment = this->get_color_attachment(read_slot);
        MTLAttachment dst_attachment = metal_fb_write->get_color_attachment(write_slot);

        if (src_attachment.used && dst_attachment.used) {

          /* TODO(Metal): Support depth(z) offset in blit if needed. */
          src_attachment.texture->blit(blit_encoder,
                                       src_x_offset,
                                       src_y_offset,
                                       0,
                                       src_attachment.slice,
                                       src_attachment.mip,
                                       dst_attachment.texture,
                                       dst_x_offset,
                                       dst_y_offset,
                                       0,
                                       dst_attachment.slice,
                                       dst_attachment.mip,
                                       width,
                                       height,
                                       1);
        } else {
          MTL_LOG_ERROR("Failed performing colour blit\n");
        }
      }
    }
    if ((do_depth || do_stencil) && blit_encoder == nil) {
      blit_encoder = mtl_context->main_command_buffer.ensure_begin_blit_encoder();
    }

    if (do_depth) {
      MTLAttachment src_attachment = this->get_depth_attachment();
      MTLAttachment dst_attachment = metal_fb_write->get_depth_attachment();

      if (src_attachment.used && dst_attachment.used) {

        /* TODO(Metal): Support depth(z) offset in blit if needed. */
        src_attachment.texture->blit(blit_encoder,
                                     src_x_offset,
                                     src_y_offset,
                                     0,
                                     src_attachment.slice,
                                     src_attachment.mip,
                                     dst_attachment.texture,
                                     dst_x_offset,
                                     dst_y_offset,
                                     0,
                                     dst_attachment.slice,
                                     dst_attachment.mip,
                                     width,
                                     height,
                                     1);
      } else {
        MTL_LOG_ERROR("Failed performing depth blit\n");
      }
    }

    /* Stencil attachment blit. */
    if (do_stencil) {
      MTLAttachment src_attachment = this->get_stencil_attachment();
      MTLAttachment dst_attachment = metal_fb_write->get_stencil_attachment();

      if (src_attachment.used && dst_attachment.used) {

        /* TODO(Metal): Support depth(z) offset in blit if needed. */
        src_attachment.texture->blit(blit_encoder,
                                     src_x_offset,
                                     src_y_offset,
                                     0,
                                     src_attachment.slice,
                                     src_attachment.mip,
                                     dst_attachment.texture,
                                     dst_x_offset,
                                     dst_y_offset,
                                     0,
                                     dst_attachment.slice,
                                     dst_attachment.mip,
                                     width,
                                     height,
                                     1);
      } else {
        MTL_LOG_ERROR("Failed performing Stencil blit\n");
      }
    }
  }

  int MTLFrameBuffer::get_width()
  {
    return m_width;
  }
  int MTLFrameBuffer::get_height()
  {
    return m_height;
  }

}  // namespace kraken::gpu
