/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2009 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup krf
 *
 * Main BlenFont (KRF) API, public functions for font handling.
 *
 * Wraps OpenGL and FreeType.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "MEM_guardedalloc.h"

#include "KLI_math.h"
#include "KLI_string.h"
#include "KLI_threads.h"

#include "KRF_api.h"

#include "IMB_colormanagement.h"

#include "GPU_matrix.h"
#include "GPU_shader.h"

#include "krf_internal.h"
#include "krf_internal_types.h"

#define KRF_RESULT_CHECK_INIT(r_info)     \
  if (r_info) {                           \
    memset(r_info, 0, sizeof(*(r_info))); \
  }                                       \
  ((void)0)

/* Font array. */
FontKRF *global_font[KRF_MAX_FONT] = {NULL};

/* XXX: should these be made into global_font_'s too? */

int krf_mono_font = -1;
int krf_mono_font_render = -1;

static FontKRF *krf_get(int fontid)
{
  if (fontid >= 0 && fontid < KRF_MAX_FONT) {
    return global_font[fontid];
  }
  return NULL;
}

int KRF_init(void)
{
  for (int i = 0; i < KRF_MAX_FONT; i++) {
    global_font[i] = NULL;
  }

  return krf_font_init();
}

void KRF_exit(void)
{
  for (int i = 0; i < KRF_MAX_FONT; i++) {
    FontKRF *font = global_font[i];
    if (font) {
      krf_font_free(font);
      global_font[i] = NULL;
    }
  }

  krf_font_exit();
}

void KRF_cache_clear(void)
{
  for (int i = 0; i < KRF_MAX_FONT; i++) {
    FontKRF *font = global_font[i];
    if (font) {
      krf_glyph_cache_clear(font);
    }
  }
}

bool krf_font_id_is_valid(int fontid)
{
  return krf_get(fontid) != NULL;
}

static int krf_search(const char *name)
{
  for (int i = 0; i < KRF_MAX_FONT; i++) {
    const FontKRF *font = global_font[i];
    if (font && (STREQ(font->name, name))) {
      return i;
    }
  }

  return -1;
}

static int krf_search_available(void)
{
  for (int i = 0; i < KRF_MAX_FONT; i++) {
    if (!global_font[i]) {
      return i;
    }
  }

  return -1;
}

bool KRF_has_glyph(int fontid, unsigned int unicode)
{
  FontKRF *font = krf_get(fontid);
  if (font) {
    return krf_get_char_index(font, unicode) != FT_Err_Ok;
  }
  return false;
}

bool KRF_is_loaded(const char *name)
{
  return krf_search(name) >= 0;
}

int KRF_load(const char *name)
{
  /* check if we already load this font. */
  int i = krf_search(name);
  if (i >= 0) {
    FontKRF *font = global_font[i];
    font->reference_count++;
    return i;
  }

  return KRF_load_unique(name);
}

int KRF_load_unique(const char *name)
{
  /* Don't search in the cache!! make a new
   * object font, this is for keep fonts threads safe.
   */
  int i = krf_search_available();
  if (i == -1) {
    printf("Too many fonts!!!\n");
    return -1;
  }

  char *filepath = krf_dir_search(name);
  if (!filepath) {
    printf("Can't find font: %s\n", name);
    return -1;
  }

  FontKRF *font = krf_font_new(name, filepath);
  MEM_freeN(filepath);

  if (!font) {
    printf("Can't load font: %s\n", name);
    return -1;
  }

  font->reference_count = 1;
  global_font[i] = font;
  return i;
}

void KRF_metrics_attach(int fontid, unsigned char *mem, int mem_size)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    krf_font_attach_from_mem(font, mem, mem_size);
  }
}

int KRF_load_mem(const char *name, const unsigned char *mem, int mem_size)
{
  int i = krf_search(name);
  if (i >= 0) {
    // font = global_font[i]; /* UNUSED */
    return i;
  }
  return KRF_load_mem_unique(name, mem, mem_size);
}

int KRF_load_mem_unique(const char *name, const unsigned char *mem, int mem_size)
{
  /*
   * Don't search in the cache, make a new object font!
   * this is to keep the font thread safe.
   */
  int i = krf_search_available();
  if (i == -1) {
    printf("Too many fonts!!!\n");
    return -1;
  }

  if (!mem_size) {
    printf("Can't load font: %s from memory!!\n", name);
    return -1;
  }

  FontKRF *font = krf_font_new_from_mem(name, mem, mem_size);
  if (!font) {
    printf("Can't load font: %s from memory!!\n", name);
    return -1;
  }

  font->reference_count = 1;
  global_font[i] = font;
  return i;
}

void KRF_unload(const char *name)
{
  for (int i = 0; i < KRF_MAX_FONT; i++) {
    FontKRF *font = global_font[i];

    if (font && (STREQ(font->name, name))) {
      KLI_assert(font->reference_count > 0);
      font->reference_count--;

      if (font->reference_count == 0) {
        krf_font_free(font);
        global_font[i] = NULL;
      }
    }
  }
}

void KRF_unload_id(int fontid)
{
  FontKRF *font = krf_get(fontid);
  if (font) {
    KLI_assert(font->reference_count > 0);
    font->reference_count--;

    if (font->reference_count == 0) {
      krf_font_free(font);
      global_font[fontid] = NULL;
    }
  }
}

void KRF_unload_all(void)
{
  for (int i = 0; i < KRF_MAX_FONT; i++) {
    FontKRF *font = global_font[i];
    if (font) {
      krf_font_free(font);
      global_font[i] = NULL;
    }
  }
  krf_mono_font = -1;
  krf_mono_font_render = -1;
  KRF_default_set(-1);
}

void KRF_enable(int fontid, int option)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->flags |= option;
  }
}

void KRF_disable(int fontid, int option)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->flags &= ~option;
  }
}

void KRF_aspect(int fontid, float x, float y, float z)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->aspect[0] = x;
    font->aspect[1] = y;
    font->aspect[2] = z;
  }
}

void KRF_matrix(int fontid, const float m[16])
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    memcpy(font->m, m, sizeof(font->m));
  }
}

void KRF_position(int fontid, float x, float y, float z)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    float xa, ya, za;
    float remainder;

    if (font->flags & KRF_ASPECT) {
      xa = font->aspect[0];
      ya = font->aspect[1];
      za = font->aspect[2];
    } else {
      xa = 1.0f;
      ya = 1.0f;
      za = 1.0f;
    }

    remainder = x - floorf(x);
    if (remainder > 0.4f && remainder < 0.6f) {
      if (remainder < 0.5f) {
        x -= 0.1f * xa;
      } else {
        x += 0.1f * xa;
      }
    }

    remainder = y - floorf(y);
    if (remainder > 0.4f && remainder < 0.6f) {
      if (remainder < 0.5f) {
        y -= 0.1f * ya;
      } else {
        y += 0.1f * ya;
      }
    }

    remainder = z - floorf(z);
    if (remainder > 0.4f && remainder < 0.6f) {
      if (remainder < 0.5f) {
        z -= 0.1f * za;
      } else {
        z += 0.1f * za;
      }
    }

    font->pos[0] = round_fl_to_int(x);
    font->pos[1] = round_fl_to_int(y);
    font->pos[2] = round_fl_to_int(z);
  }
}

void KRF_size(int fontid, float size)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    krf_font_size(font, size);
  }
}

#if KRF_BLUR_ENABLE
void KRF_blur(int fontid, int size)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->blur = size;
  }
}
#endif

void KRF_color4ubv(int fontid, const unsigned char rgba[4])
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->color[0] = rgba[0];
    font->color[1] = rgba[1];
    font->color[2] = rgba[2];
    font->color[3] = rgba[3];
  }
}

void KRF_color3ubv_alpha(int fontid, const unsigned char rgb[3], unsigned char alpha)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->color[0] = rgb[0];
    font->color[1] = rgb[1];
    font->color[2] = rgb[2];
    font->color[3] = alpha;
  }
}

void KRF_color3ubv(int fontid, const unsigned char rgb[3])
{
  KRF_color3ubv_alpha(fontid, rgb, 255);
}

void KRF_color4ub(int fontid,
                  unsigned char r,
                  unsigned char g,
                  unsigned char b,
                  unsigned char alpha)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->color[0] = r;
    font->color[1] = g;
    font->color[2] = b;
    font->color[3] = alpha;
  }
}

void KRF_color3ub(int fontid, unsigned char r, unsigned char g, unsigned char b)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->color[0] = r;
    font->color[1] = g;
    font->color[2] = b;
    font->color[3] = 255;
  }
}

void KRF_color4fv(int fontid, const float rgba[4])
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    rgba_float_to_uchar(font->color, rgba);
  }
}

void KRF_color4f(int fontid, float r, float g, float b, float a)
{
  const float rgba[4] = {r, g, b, a};
  KRF_color4fv(fontid, rgba);
}

void KRF_color3fv_alpha(int fontid, const float rgb[3], float alpha)
{
  float rgba[4];
  copy_v3_v3(rgba, rgb);
  rgba[3] = alpha;
  KRF_color4fv(fontid, rgba);
}

void KRF_color3f(int fontid, float r, float g, float b)
{
  const float rgba[4] = {r, g, b, 1.0f};
  KRF_color4fv(fontid, rgba);
}

void KRF_batch_draw_begin(void)
{
  KLI_assert(g_batch.enabled == false);
  g_batch.enabled = true;
}

void KRF_batch_draw_flush(void)
{
  if (g_batch.enabled) {
    krf_batch_draw();
  }
}

void KRF_batch_draw_end(void)
{
  KLI_assert(g_batch.enabled == true);
  krf_batch_draw(); /* Draw remaining glyphs */
  g_batch.enabled = false;
}

static void krf_draw_gl__start(const FontKRF *font)
{
  /*
   * The pixmap alignment hack is handle
   * in KRF_position (old ui_rasterpos_safe).
   */

  if ((font->flags & (KRF_ROTATION | KRF_MATRIX | KRF_ASPECT)) == 0) {
    return; /* glyphs will be translated individually and batched. */
  }

  GPU_matrix_push();

  if (font->flags & KRF_MATRIX) {
    GPU_matrix_mul(font->m);
  }

  GPU_matrix_translate_3f(font->pos[0], font->pos[1], font->pos[2]);

  if (font->flags & KRF_ASPECT) {
    GPU_matrix_scale_3fv(font->aspect);
  }

  if (font->flags & KRF_ROTATION) {
    GPU_matrix_rotate_2d(RAD2DEG(font->angle));
  }
}

static void krf_draw_gl__end(const FontKRF *font)
{
  if ((font->flags & (KRF_ROTATION | KRF_MATRIX | KRF_ASPECT)) != 0) {
    GPU_matrix_pop();
  }
}

void KRF_draw_ex(int fontid, const char *str, const size_t str_len, struct ResultKRF *r_info)
{
  FontKRF *font = krf_get(fontid);

  KRF_RESULT_CHECK_INIT(r_info);

  if (font) {
    krf_draw_gl__start(font);
    if (font->flags & KRF_WORD_WRAP) {
      krf_font_draw__wrap(font, str, str_len, r_info);
    } else {
      krf_font_draw(font, str, str_len, r_info);
    }
    krf_draw_gl__end(font);
  }
}
void KRF_draw(int fontid, const char *str, const size_t str_len)
{
  if (str_len == 0 || str[0] == '\0') {
    return;
  }

  /* Avoid kgl usage to corrupt KRF drawing. */
  GPU_kgl_end();

  KRF_draw_ex(fontid, str, str_len, NULL);
}

int KRF_draw_mono(int fontid, const char *str, const size_t str_len, int cwidth)
{
  if (str_len == 0 || str[0] == '\0') {
    return 0;
  }

  FontKRF *font = krf_get(fontid);
  int columns = 0;

  if (font) {
    krf_draw_gl__start(font);
    columns = krf_font_draw_mono(font, str, str_len, cwidth);
    krf_draw_gl__end(font);
  }

  return columns;
}

void KRF_boundbox_foreach_glyph(int fontid,
                                const char *str,
                                const size_t str_len,
                                KRF_GlyphBoundsFn user_fn,
                                void *user_data)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    if (font->flags & KRF_WORD_WRAP) {
      /* TODO: word-wrap support. */
      KLI_assert(0);
    } else {
      krf_font_boundbox_foreach_glyph(font, str, str_len, user_fn, user_data);
    }
  }
}

size_t KRF_str_offset_from_cursor_position(int fontid,
                                           const char *str,
                                           size_t str_len,
                                           int location_x)
{
  FontKRF *font = krf_get(fontid);
  if (font) {
    return krf_str_offset_from_cursor_position(font, str, str_len, location_x);
  }
  return 0;
}

bool BLF_str_offset_to_glyph_bounds(int fontid,
                                    const char *str,
                                    size_t str_offset,
                                    rcti *glyph_bounds)
{
  FontKRF *font = krf_get(fontid);
  if (font) {
    krf_str_offset_to_glyph_bounds(font, str, str_offset, glyph_bounds);
    return true;
  }
  return false;
}

size_t KRF_width_to_strlen(int fontid,
                           const char *str,
                           const size_t str_len,
                           float width,
                           float *r_width)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    const float xa = (font->flags & KRF_ASPECT) ? font->aspect[0] : 1.0f;
    size_t ret;
    int width_result;
    ret = krf_font_width_to_strlen(font, str, str_len, width / xa, &width_result);
    if (r_width) {
      *r_width = (float)width_result * xa;
    }
    return ret;
  }

  if (r_width) {
    *r_width = 0.0f;
  }
  return 0;
}

size_t KRF_width_to_rstrlen(int fontid,
                            const char *str,
                            const size_t str_len,
                            float width,
                            float *r_width)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    const float xa = (font->flags & KRF_ASPECT) ? font->aspect[0] : 1.0f;
    size_t ret;
    int width_result;
    ret = krf_font_width_to_rstrlen(font, str, str_len, width / xa, &width_result);
    if (r_width) {
      *r_width = (float)width_result * xa;
    }
    return ret;
  }

  if (r_width) {
    *r_width = 0.0f;
  }
  return 0;
}

void KRF_boundbox_ex(int fontid,
                     const char *str,
                     const size_t str_len,
                     rcti *r_box,
                     struct ResultKRF *r_info)
{
  FontKRF *font = krf_get(fontid);

  KRF_RESULT_CHECK_INIT(r_info);

  if (font) {
    if (font->flags & KRF_WORD_WRAP) {
      krf_font_boundbox__wrap(font, str, str_len, r_box, r_info);
    } else {
      krf_font_boundbox(font, str, str_len, r_box, r_info);
    }
  }
}

void KRF_boundbox(int fontid, const char *str, const size_t str_len, rcti *r_box)
{
  KRF_boundbox_ex(fontid, str, str_len, r_box, NULL);
}

void KRF_width_and_height(int fontid,
                          const char *str,
                          const size_t str_len,
                          float *r_width,
                          float *r_height)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    krf_font_width_and_height(font, str, str_len, r_width, r_height, NULL);
  } else {
    *r_width = *r_height = 0.0f;
  }
}

float KRF_width_ex(int fontid, const char *str, const size_t str_len, struct ResultKRF *r_info)
{
  FontKRF *font = krf_get(fontid);

  KRF_RESULT_CHECK_INIT(r_info);

  if (font) {
    return krf_font_width(font, str, str_len, r_info);
  }

  return 0.0f;
}

float KRF_width(int fontid, const char *str, const size_t str_len)
{
  return KRF_width_ex(fontid, str, str_len, NULL);
}

float KRF_fixed_width(int fontid)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    return krf_font_fixed_width(font);
  }

  return 0.0f;
}

float KRF_height_ex(int fontid, const char *str, const size_t str_len, struct ResultKRF *r_info)
{
  FontKRF *font = krf_get(fontid);

  KRF_RESULT_CHECK_INIT(r_info);

  if (font) {
    return krf_font_height(font, str, str_len, r_info);
  }

  return 0.0f;
}

float KRF_height(int fontid, const char *str, const size_t str_len)
{
  return KRF_height_ex(fontid, str, str_len, NULL);
}

int KRF_height_max(int fontid)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    return krf_font_height_max(font);
  }

  return 0;
}

int KRF_width_max(int fontid)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    return krf_font_width_max(font);
  }

  return 0;
}

int KRF_descender(int fontid)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    return krf_font_descender(font);
  }

  return 0;
}

int KRF_ascender(int fontid)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    return krf_font_ascender(font);
  }

  return 0.0f;
}

void KRF_rotation(int fontid, float angle)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->angle = angle;
  }
}

void KRF_clipping(int fontid, int xmin, int ymin, int xmax, int ymax)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->clip_rec.xmin = xmin;
    font->clip_rec.ymin = ymin;
    font->clip_rec.xmax = xmax;
    font->clip_rec.ymax = ymax;
  }
}

void KRF_wordwrap(int fontid, int wrap_width)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->wrap_width = wrap_width;
  }
}

void KRF_shadow(int fontid, int level, const float rgba[4])
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->shadow = level;
    rgba_float_to_uchar(font->shadow_color, rgba);
  }
}

void KRF_shadow_offset(int fontid, int x, int y)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->shadow_x = x;
    font->shadow_y = y;
  }
}

void KRF_buffer(int fontid,
                float *fbuf,
                unsigned char *cbuf,
                int w,
                int h,
                int nch,
                struct ColorManagedDisplay *display)
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    font->buf_info.fbuf = fbuf;
    font->buf_info.cbuf = cbuf;
    font->buf_info.dims[0] = w;
    font->buf_info.dims[1] = h;
    font->buf_info.ch = nch;
    font->buf_info.display = display;
  }
}

void KRF_buffer_col(int fontid, const float rgba[4])
{
  FontKRF *font = krf_get(fontid);

  if (font) {
    copy_v4_v4(font->buf_info.col_init, rgba);
  }
}

void krf_draw_buffer__start(FontKRF *font)
{
  FontBufInfoKRF *buf_info = &font->buf_info;

  rgba_float_to_uchar(buf_info->col_char, buf_info->col_init);

  if (buf_info->display) {
    copy_v4_v4(buf_info->col_float, buf_info->col_init);
    IMB_colormanagement_display_to_scene_linear_v3(buf_info->col_float, buf_info->display);
  } else {
    srgb_to_linearrgb_v4(buf_info->col_float, buf_info->col_init);
  }
}
void krf_draw_buffer__end(void) {}

void KRF_draw_buffer_ex(int fontid,
                        const char *str,
                        const size_t str_len,
                        struct ResultKRF *r_info)
{
  FontKRF *font = krf_get(fontid);

  if (font && (font->buf_info.fbuf || font->buf_info.cbuf)) {
    krf_draw_buffer__start(font);
    if (font->flags & KRF_WORD_WRAP) {
      krf_font_draw_buffer__wrap(font, str, str_len, r_info);
    } else {
      krf_font_draw_buffer(font, str, str_len, r_info);
    }
    krf_draw_buffer__end();
  }
}
void KRF_draw_buffer(int fontid, const char *str, const size_t str_len)
{
  KRF_draw_buffer_ex(fontid, str, str_len, NULL);
}

char *KRF_display_name_from_file(const char *filepath)
{
  /* While listing font directories this function can be called simultaneously from a greater
   * number of threads than we want the FreeType cache to keep open at a time. Therefore open
   * with own FT_Library object and use FreeType calls directly to avoid any contention. */
  char *name = NULL;
  FT_Library ft_library;
  if (FT_Init_FreeType(&ft_library) == FT_Err_Ok) {
    FT_Face face;
    if (FT_New_Face(ft_library, filepath, 0, &face) == FT_Err_Ok) {
      if (face->family_name) {
        name = KLI_sprintfN("%s %s", face->family_name, face->style_name);
      }
      FT_Done_Face(face);
    }
    FT_Done_FreeType(ft_library);
  }
  return name;
}

#ifdef DEBUG
void KRF_state_print(int fontid)
{
  FontKRF *font = krf_get(fontid);
  if (font) {
    printf("fontid %d %p\n", fontid, (void *)font);
    printf("  name:    '%s'\n", font->name);
    printf("  size:     %f\n", font->size);
    printf("  pos:      %d %d %d\n", UNPACK3(font->pos));
    printf("  aspect:   (%d) %.6f %.6f %.6f\n",
           (font->flags & KRF_ROTATION) != 0,
           UNPACK3(font->aspect));
    printf("  angle:    (%d) %.6f\n", (font->flags & KRF_ASPECT) != 0, font->angle);
    printf("  flag:     %d\n", font->flags);
  } else {
    printf("fontid %d (NULL)\n", fontid);
  }
  fflush(stdout);
}
#endif
