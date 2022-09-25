/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2009 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup krf
 */

#pragma once

struct FontKRF;
struct GlyphKRF;
struct GlyphCacheKRF;
struct ResultKRF;
struct rctf;
struct rcti;

/* Max number of FontKRFs in memory. Take care that every font has a glyph cache per size/dpi,
 * so we don't need load the same font with different size, just load one and call KRF_size. */
#define KRF_MAX_FONT 64

/* Maximum number of opened FT_Face objects managed by cache. 0 is default of 2. */
#define KRF_CACHE_MAX_FACES 4
/* Maximum number of opened FT_Size objects managed by cache. 0 is default of 4 */
#define KRF_CACHE_MAX_SIZES 8
/* Maximum number of bytes to use for cached data nodes. 0 is default of 200,000. */
#define KRF_CACHE_BYTES 400000

extern struct FontKRF *global_font[KRF_MAX_FONT];

void krf_batch_draw_begin(struct FontKRF *font);
void krf_batch_draw(void);

unsigned int krf_next_p2(unsigned int x);
unsigned int krf_hash(unsigned int val);

char *krf_dir_search(const char *file);
/**
 * Some font have additional file with metrics information,
 * in general, the extension of the file is: `.afm` or `.pfm`
 */
char *krf_dir_metrics_search(const char *filepath);
/* int krf_dir_split(const char *str, char *file, int *size); */ /* UNUSED */

int krf_font_init(void);
void krf_font_exit(void);

bool krf_font_id_is_valid(int fontid);

/**
 * Return glyph id from char-code.
 */
uint krf_get_char_index(struct FontKRF *font, uint charcode);

bool krf_ensure_face(struct FontKRF *font);
void krf_ensure_size(struct FontKRF *font);

void krf_draw_buffer__start(struct FontKRF *font);
void krf_draw_buffer__end(void);

struct FontKRF *krf_font_new_ex(const char *name,
                                const char *filepath,
                                const unsigned char *mem,
                                size_t mem_size,
                                void *ft_library);

struct FontKRF *krf_font_new(const char *name, const char *filepath);
struct FontKRF *krf_font_new_from_mem(const char *name, const unsigned char *mem, size_t mem_size);
void krf_font_attach_from_mem(struct FontKRF *font, const unsigned char *mem, size_t mem_size);

/**
 * Change font's output size. Returns true if successful in changing the size.
 */
bool krf_font_size(struct FontKRF *font, float size, unsigned int dpi);

void krf_font_draw(struct FontKRF *font,
                   const char *str,
                   size_t str_len,
                   struct ResultKRF *r_info);
void krf_font_draw__wrap(struct FontKRF *font,
                         const char *str,
                         size_t str_len,
                         struct ResultKRF *r_info);

/**
 * Use fixed column width, but an utf8 character may occupy multiple columns.
 */
int krf_font_draw_mono(struct FontKRF *font, const char *str, size_t str_len, int cwidth);
void krf_font_draw_buffer(struct FontKRF *font,
                          const char *str,
                          size_t str_len,
                          struct ResultKRF *r_info);
void krf_font_draw_buffer__wrap(struct FontKRF *font,
                                const char *str,
                                size_t str_len,
                                struct ResultKRF *r_info);
size_t krf_font_width_to_strlen(
    struct FontKRF *font, const char *str, size_t str_len, int width, int *r_width);
size_t krf_font_width_to_rstrlen(
    struct FontKRF *font, const char *str, size_t str_len, int width, int *r_width);
void krf_font_boundbox(struct FontKRF *font,
                       const char *str,
                       size_t str_len,
                       struct rcti *r_box,
                       struct ResultKRF *r_info);
void krf_font_boundbox__wrap(struct FontKRF *font,
                             const char *str,
                             size_t str_len,
                             struct rcti *r_box,
                             struct ResultKRF *r_info);
void krf_font_width_and_height(struct FontKRF *font,
                               const char *str,
                               size_t str_len,
                               float *r_width,
                               float *r_height,
                               struct ResultKRF *r_info);
float krf_font_width(struct FontKRF *font,
                     const char *str,
                     size_t str_len,
                     struct ResultKRF *r_info);
float krf_font_height(struct FontKRF *font,
                      const char *str,
                      size_t str_len,
                      struct ResultKRF *r_info);
float krf_font_fixed_width(struct FontKRF *font);
int krf_font_height_max(struct FontKRF *font);
int krf_font_width_max(struct FontKRF *font);
int krf_font_descender(struct FontKRF *font);
int krf_font_ascender(struct FontKRF *font);

char *krf_display_name(struct FontKRF *font);

void krf_font_boundbox_foreach_glyph(struct FontKRF *font,
                                     const char *str,
                                     size_t str_len,
                                     bool (*user_fn)(const char *str,
                                                     size_t str_step_ofs,
                                                     const struct rcti *glyph_step_bounds,
                                                     int glyph_advance_x,
                                                     const struct rcti *glyph_bounds,
                                                     const int glyph_bearing[2],
                                                     void *user_data),
                                     void *user_data,
                                     struct ResultKRF *r_info);

int krf_font_count_missing_chars(struct FontKRF *font,
                                 const char *str,
                                 size_t str_len,
                                 int *r_tot_chars);

void krf_font_free(struct FontKRF *font);

struct GlyphCacheKRF *krf_glyph_cache_acquire(struct FontKRF *font);
void krf_glyph_cache_release(struct FontKRF *font);
void krf_glyph_cache_clear(struct FontKRF *font);

/**
 * Create (or load from cache) a fully-rendered bitmap glyph.
 */
struct GlyphKRF *krf_glyph_ensure(struct FontKRF *font, struct GlyphCacheKRF *gc, uint charcode);

void krf_glyph_free(struct GlyphKRF *g);
void krf_glyph_draw(
    struct FontKRF *font, struct GlyphCacheKRF *gc, struct GlyphKRF *g, int x, int y);

#ifdef WIN32
/* krf_font_win32_compat.c */

#  ifdef FT_FREETYPE_H
extern FT_Error FT_New_Face__win32_compat(FT_Library library,
                                          const char *pathname,
                                          FT_Long face_index,
                                          FT_Face *aface);
#  endif
#endif
