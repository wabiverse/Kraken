/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2009 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup krf
 */

#pragma once

#include "KLI_compiler_attrs.h"
#include "KLI_sys_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name of subfolder inside KRAKEN_DATAFILES that contains font files. */
#define KRF_DATAFILES_FONTS_DIR "fonts"

/* File name of the default variable-width font. */
#define KRF_DEFAULT_PROPORTIONAL_FONT "DejaVuSans.woff2"

/* File name of the default fixed-pitch font. */
#define KRF_DEFAULT_MONOSPACED_FONT "DejaVuSansMono.woff2"

/* enable this only if needed (unused circa 2016) */
#define KRF_BLUR_ENABLE 0

struct ColorManagedDisplay;
struct ResultKRF;
struct rctf;
struct rcti;

int KRF_init(void);
void KRF_exit(void);

void KRF_cache_clear(void);

/**
 * Optional cache flushing function, called before #krf_batch_draw.
 */
void KRF_cache_flush_set_fn(void (*cache_flush_fn)(void));

/**
 * Loads a font, or returns an already loaded font and increments its reference count.
 */
int KRF_load(const char *name) ATTR_NONNULL();
int KRF_load_mem(const char *name, const unsigned char *mem, int mem_size) ATTR_NONNULL();
bool KRF_is_loaded(const char *name) ATTR_NONNULL();

int KRF_load_unique(const char *name) ATTR_NONNULL();
int KRF_load_mem_unique(const char *name, const unsigned char *mem, int mem_size) ATTR_NONNULL();

void KRF_unload(const char *name) ATTR_NONNULL();
void KRF_unload_id(int fontid);
void KRF_unload_all(void);

char *KRF_display_name_from_file(const char *filepath);

/**
 * Check if font supports a particular glyph.
 */
bool KRF_has_glyph(int fontid, unsigned int unicode);

/**
 * Attach a file with metrics information from memory.
 */
void KRF_metrics_attach(int fontid, unsigned char *mem, int mem_size);

void KRF_aspect(int fontid, float x, float y, float z);
void KRF_position(int fontid, float x, float y, float z);
void KRF_size(int fontid, float size);

/* Goal: small but useful color API. */

void KRF_color4ubv(int fontid, const unsigned char rgba[4]);
void KRF_color3ubv(int fontid, const unsigned char rgb[3]);
void KRF_color3ubv_alpha(int fontid, const unsigned char rgb[3], unsigned char alpha);
void KRF_color4ub(int fontid,
                  unsigned char r,
                  unsigned char g,
                  unsigned char b,
                  unsigned char alpha);
void KRF_color3ub(int fontid, unsigned char r, unsigned char g, unsigned char b);
void KRF_color4f(int fontid, float r, float g, float b, float a);
void KRF_color4fv(int fontid, const float rgba[4]);
void KRF_color3f(int fontid, float r, float g, float b);
void KRF_color3fv_alpha(int fontid, const float rgb[3], float alpha);
/* Also available: `UI_FontThemeColor(fontid, colorid)`. */

/**
 * Set a 4x4 matrix to be multiplied before draw the text.
 * Remember that you need call KRF_enable(KRF_MATRIX)
 * to enable this.
 *
 * The order of the matrix is like GL:
 * \code{.unparsed}
 *  | m[0]  m[4]  m[8]  m[12] |
 *  | m[1]  m[5]  m[9]  m[13] |
 *  | m[2]  m[6]  m[10] m[14] |
 *  | m[3]  m[7]  m[11] m[15] |
 * \endcode
 */
void KRF_matrix(int fontid, const float m[16]);

/**
 * Batch draw-calls together as long as
 * the model-view matrix and the font remain unchanged.
 */
void KRF_batch_draw_begin(void);
void KRF_batch_draw_flush(void);
void KRF_batch_draw_end(void);

/**
 * Draw the string using the current font.
 */
void KRF_draw_ex(int fontid, const char *str, size_t str_len, struct ResultKRF *r_info)
  ATTR_NONNULL(2);
void KRF_draw(int fontid, const char *str, size_t str_len) ATTR_NONNULL(2);
int KRF_draw_mono(int fontid, const char *str, size_t str_len, int cwidth) ATTR_NONNULL(2);

typedef bool (*KRF_GlyphBoundsFn)(const char *str,
                                  size_t str_step_ofs,
                                  const struct rcti *bounds,
                                  void *user_data);

/**
 * Run @a user_fn for each character, with the bound-box that would be used for drawing.
 *
 * @param user_fn: Callback that runs on each glyph, returning false early exits.
 * @param user_data: User argument passed to @a user_fn.
 *
 * @note The font position, clipping, matrix and rotation are not applied.
 */
void KRF_boundbox_foreach_glyph(int fontid,
                                const char *str,
                                size_t str_len,
                                KRF_GlyphBoundsFn user_fn,
                                void *user_data) ATTR_NONNULL(2);

/**
 * Get the byte offset within a string, selected by mouse at a horizontal location.
 */
size_t KRF_str_offset_from_cursor_position(int fontid,
                                           const char *str,
                                           size_t str_len,
                                           int location_x);

/**
 * Return bounds of the glyph rect at the string offset.
 */
bool KRF_str_offset_to_glyph_bounds(int fontid,
                                    const char *str,
                                    size_t str_offset,
                                    struct rcti *glyph_bounds);

/**
 * Get the string byte offset that fits within a given width.
 */
size_t KRF_width_to_strlen(int fontid,
                           const char *str,
                           size_t str_len,
                           float width,
                           float *r_width) ATTR_NONNULL(2);
/**
 * Same as KRF_width_to_strlen but search from the string end.
 */
size_t KRF_width_to_rstrlen(int fontid,
                            const char *str,
                            size_t str_len,
                            float width,
                            float *r_width) ATTR_NONNULL(2);

/**
 * This function return the bounding box of the string
 * and are not multiplied by the aspect.
 */
void KRF_boundbox_ex(int fontid,
                     const char *str,
                     size_t str_len,
                     struct rcti *box,
                     struct ResultKRF *r_info) ATTR_NONNULL(2);
void KRF_boundbox(int fontid, const char *str, size_t str_len, struct rcti *box) ATTR_NONNULL();

/**
 * The next both function return the width and height
 * of the string, using the current font and both value
 * are multiplied by the aspect of the font.
 */
float KRF_width_ex(int fontid, const char *str, size_t str_len, struct ResultKRF *r_info)
  ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(2);
float KRF_width(int fontid, const char *str, size_t str_len) ATTR_WARN_UNUSED_RESULT
  ATTR_NONNULL();
float KRF_height_ex(int fontid, const char *str, size_t str_len, struct ResultKRF *r_info)
  ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(2);
float KRF_height(int fontid, const char *str, size_t str_len) ATTR_WARN_UNUSED_RESULT
  ATTR_NONNULL();

/**
 * Return dimensions of the font without any sample text.
 */
int KRF_height_max(int fontid) ATTR_WARN_UNUSED_RESULT;
int KRF_width_max(int fontid) ATTR_WARN_UNUSED_RESULT;
int KRF_descender(int fontid) ATTR_WARN_UNUSED_RESULT;
int KRF_ascender(int fontid) ATTR_WARN_UNUSED_RESULT;

/**
 * The following function return the width and height of the string, but
 * just in one call, so avoid extra freetype2 stuff.
 */
void KRF_width_and_height(int fontid,
                          const char *str,
                          size_t str_len,
                          float *r_width,
                          float *r_height) ATTR_NONNULL();

/**
 * For fixed width fonts only, returns the width of a
 * character.
 */
float KRF_fixed_width(int fontid) ATTR_WARN_UNUSED_RESULT;

/**
 * By default, rotation and clipping are disable and
 * have to be enable/disable using KRF_enable/disable.
 */
void KRF_rotation(int fontid, float angle);
void KRF_clipping(int fontid, int xmin, int ymin, int xmax, int ymax);
void KRF_wordwrap(int fontid, int wrap_width);

#if KRF_BLUR_ENABLE
void KRF_blur(int fontid, int size);
#endif

void KRF_enable(int fontid, int option);
void KRF_disable(int fontid, int option);

/**
 * Shadow options, level is the blur level, can be 3, 5 or 0 and
 * the other argument are the RGBA color.
 * Take care that shadow need to be enable using #KRF_enable!
 */
void KRF_shadow(int fontid, int level, const float rgba[4]) ATTR_NONNULL(3);

/**
 * Set the offset for shadow text, this is the current cursor
 * position plus this offset, don't need call KRF_position before
 * this function, the current position is calculate only on
 * KRF_draw, so it's safe call this whenever you like.
 */
void KRF_shadow_offset(int fontid, int x, int y);

/**
 * Set the buffer, size and number of channels to draw, one thing to take care is call
 * this function with NULL pointer when we finish, for example:
 * \code{.c}
 * KRF_buffer(my_fbuf, my_cbuf, 100, 100, 4, true, NULL);
 *
 * ... set color, position and draw ...
 *
 * KRF_buffer(NULL, NULL, NULL, 0, 0, false, NULL);
 * \endcode
 */
void KRF_buffer(int fontid,
                float *fbuf,
                unsigned char *cbuf,
                int w,
                int h,
                int nch,
                struct ColorManagedDisplay *display);

/**
 * Set the color to be used for text.
 */
void KRF_buffer_col(int fontid, const float rgba[4]) ATTR_NONNULL(2);

/**
 * Draw the string into the buffer, this function draw in both buffer,
 * float and unsigned char _BUT_ it's not necessary set both buffer, NULL is valid here.
 */
void KRF_draw_buffer_ex(int fontid, const char *str, size_t str_len, struct ResultKRF *r_info)
  ATTR_NONNULL(2);
void KRF_draw_buffer(int fontid, const char *str, size_t str_len) ATTR_NONNULL(2);

/**
 * Add a path to the font dir paths.
 */
void KRF_dir_add(const char *path) ATTR_NONNULL();

/**
 * Remove a path from the font dir paths.
 */
void KRF_dir_rem(const char *path) ATTR_NONNULL();

/**
 * Return an array with all the font dir (this can be used for file-selector).
 */
char **KRF_dir_get(int *ndir) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

/**
 * Free the data return by #KRF_dir_get.
 */
void KRF_dir_free(char **dirs, int count) ATTR_NONNULL();

/* krf_thumbs.c */

/**
 * This function is used for generating thumbnail previews.
 *
 * @note called from a thread, so it bypasses the normal KRF_* api (which isn't thread-safe).
 */
bool KRF_thumb_preview(const char *filename, unsigned char *buf, int w, int h, int channels)
  ATTR_NONNULL();

/* krf_default.c */

void KRF_default_size(float size);
void KRF_default_set(int fontid);
/**
 * Get default font ID so we can pass it to other functions.
 */
int KRF_default(void);
/**
 * Draw the string using the default font, size and DPI.
 */
void KRF_draw_default(float x, float y, float z, const char *str, size_t str_len) ATTR_NONNULL();
/**
 * Set size and DPI, and return default font ID.
 */
int KRF_set_default(void);

/* krf_font_default.c */

int KRF_load_default(bool unique);
int KRF_load_mono_default(bool unique);
void KRF_load_font_stack(void);

#ifdef DEBUG
void KRF_state_print(int fontid);
#endif

/** #FontKRF.flags. */
enum
{
  KRF_ROTATION = 1 << 0,
  KRF_CLIPPING = 1 << 1,
  KRF_SHADOW = 1 << 2,
  // KRF_FLAG_UNUSED_3 = 1 << 3, /* dirty */
  KRF_MATRIX = 1 << 4,
  KRF_ASPECT = 1 << 5,
  KRF_WORD_WRAP = 1 << 6,
  /** No anti-aliasing. */
  KRF_MONOCHROME = 1 << 7,
  KRF_HINTING_NONE = 1 << 8,
  KRF_HINTING_SLIGHT = 1 << 9,
  KRF_HINTING_FULL = 1 << 10,
  KRF_BOLD = 1 << 11,
  KRF_ITALIC = 1 << 12,
  /** Intended USE is monospaced, regardless of font type. */
  KRF_MONOSPACED = 1 << 13,
  /** A font within the default stack of fonts. */
  KRF_DEFAULT = 1 << 14,
  /** Must only be used as last font in the stack. */
  KRF_LAST_RESORT = 1 << 15,
  /** Failure to load this font. Don't try again. */
  KRF_BAD_FONT = 1 << 16,
  /** This font is managed by the FreeType cache subsystem. */
  KRF_CACHED = 1 << 17,
};

#define KRF_DRAW_STR_DUMMY_MAX 1024

/* XXX, bad design */
extern int krf_mono_font;
extern int krf_mono_font_render; /* don't mess drawing with render threads. */

/**
 * Result of drawing/evaluating the string
 */
struct ResultKRF
{
  /**
   * Number of lines drawn when #KRF_WORD_WRAP is enabled (both wrapped and `\n` newline).
   */
  int lines;
  /**
   * The 'cursor' position on completion (ignoring character boundbox).
   */
  int width;
};

#ifdef __cplusplus
}
#endif
